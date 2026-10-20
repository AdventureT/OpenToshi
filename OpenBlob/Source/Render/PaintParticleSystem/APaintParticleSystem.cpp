#include "pch.h"
#include "APaintParticleSystem.h"

#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

#include <GUI/T2GUITextBox.h>

#include <DirectXMath.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderContext_DX11.h>
#include <Render/TRenderContext.h>
#include <Render/TRenderPacket.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include "ARandom.h"
#include "Collision/ACollisionInstance2.h"
#include "Render/Decal/ADecalManager.h"
#include <Render/TViewport.h>
#include <Math/TMathInline.h>

#include <cstring>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The shaders (0x896c70 and 0x896ef8 point to them)
// 0x896c70
static const TCHAR* s_szPaintVS =
    "struct VS_IN { float3 ObjPos : POSITION; float4 Color : COLOR; }; struct VS_OUT { float4 ProjPos : SV_POSITI"
    "ON; float4 Color : COLOR; }; float4x4 matWVP : register(c0); float4x4 matWV : register(c4); float3x3 matNorm"
    " : register(c8); float4x4 texMtx0 : register(c12); float4x4 texMtx1 : register(c16); float4x4 texMtx2 : regi"
    "ster(c20); float4x4 texMtx3 : register(c24); float4x4 texMtx6 : register(c28); float4x4 texMtx7 : register(c"
    "32); float4x3 bones[30] : register(c40); float3x3 bonesNrm[30] : register(c130); VS_OUT main(VS_IN In) { VS_"
    "OUT Out; Out.ProjPos = mul(matWVP, float4(In.ObjPos, 1.0)); Out.Color = In.Color; return Out; }";

// 0x896ef8
static const TCHAR* s_szPaintPS =
    "struct VS_OUT { float4 ProjPos : SV_POSITION; float4 Color : COLOR; }; uniform sampler2D texture0 : register"
    "(s0); uniform sampler2D texture1 : register(s1); uniform sampler2D texture2 : register(s2); uniform sampler2"
    "D texture3 : register(s3); float4 alphaTest : register(c0); float4 kColor0 : register(c1); float4 kColor1 : "
    "register(c2); float4 kColor2 : register(c3); float4 kColor3 : register(c4); float4 tevReg0 : register(c5); f"
    "loat4 tevReg1 : register(c6); float4 tevReg2 : register(c7); float4 chanMatColor0 : register(c8); float4 cha"
    "nMatColor1 : register(c9); float4 chanAmbColor0 : register(c10); float4 chanAmbColor1 : register(c11); float"
    "4 light0Pos : register(c12); float4 light0Color : register(c13); float4 light6Pos : register(c14); float4 li"
    "ght6Color : register(c15); float4 light7Pos : register(c16); float4 light7Color : register(c17); float4 main"
    "(VS_OUT In) : SV_TARGET { float4 result = In.Color; return result; }";

APaintParticleSystem* APaintParticleSystem::s_Instance = TNULL;

//-----------------------------------------------------------------------------
// DummyMaterial
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005fb6e0
APaintParticleSystem::DummyMaterial::DummyMaterial()
{
}

// $deBlob: FUNCTION 005fb970
APaintParticleSystem::DummyMaterial::~DummyMaterial()
{
}

//-----------------------------------------------------------------------------
// DummyMesh
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005fb700
APaintParticleSystem::DummyMesh::DummyMesh()
{
}

// $deBlob: FUNCTION 005fb9a0
APaintParticleSystem::DummyMesh::~DummyMesh()
{
}

// $deBlob: FUNCTION 005fbad0
void APaintParticleSystem::DummyMesh::OnDestroy()
{
	m_State &= ~TMesh::State_Created;
}

//-----------------------------------------------------------------------------
// APaintParticleSystem
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 005fa060
APaintParticleSystem::APaintParticleSystem( TFLOAT a_fGravity )
{
	TASSERT( s_Instance == TNULL );

	m_bEnabled      = TTRUE;
	m_fGravity      = a_fGravity;
	m_bDirty        = TFALSE;
	m_fFadeTimer    = 0.0f;
	m_iLineCursor   = 0;
	m_iLineCount    = 0;
	m_pVertexShader = TNULL;
	m_pInputLayout  = TNULL;
	m_pPixelShader  = TNULL;

	m_mMatrix.Identity();

	m_pVertexBuffer = TRenderDX11::Interface()->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, 6400, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE );
	m_pPaintBufferA = TMemalign( 16, 4800 );
	m_pPaintBufferB = TMemalign( 16, 4800 );
	m_pPaintCells   = TREINTERPRETCAST( TUINT32*, TMemalign( 16, 1600 ) );
	if ( m_pPaintCells != TNULL )
		std::memset( m_pPaintCells, 0, 1600 );

	m_pLineIndices = TREINTERPRETCAST( TUINT16*, TMemalign( 16, 400 ) );

	s_Instance = this;
}

// $deBlob: FUNCTION 005fb940
// $deBlob: FUNCTION 005fa2b0
APaintParticleSystem::~APaintParticleSystem()
{
	if ( m_pVertexShader != TNULL )
	{
		m_pVertexShader->Release();
		m_pVertexShader = TNULL;
	}

	if ( m_pInputLayout != TNULL )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}

	if ( m_pPixelShader != TNULL )
	{
		m_pPixelShader->Release();
		m_pPixelShader = TNULL;
	}

	if ( m_pVertexBuffer != TNULL )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = TNULL;
	}

	TFree( m_pPaintBufferA );
	TFree( m_pPaintBufferB );
	TFree( m_pPaintCells );
	TFree( m_pLineIndices );

	s_Instance = TNULL;
}

// $deBlob: FUNCTION 005fac10
void APaintParticleSystem::OnDestroy()
{
	TOrderTable::DeregisterMaterial( m_DummyMaterial.m_pRegMaterial );
	m_DummyMesh.Invalidate();
	m_DummyMesh.OnDestroy();
	m_DummyMaterial.OnDestroy();

	m_lFreeList.Clear();
	m_lUsedList.Clear();
	m_lDecalList.Clear();

	TShader::OnDestroy();
}

// $deBlob: FUNCTION 005fbbf0
void APaintParticleSystem::StartFlush()
{
}

// $deBlob: FUNCTION 005fba30
void APaintParticleSystem::EndFlush()
{
}

// $deBlob: FUNCTION 005fba40
const TCHAR* APaintParticleSystem::GetShaderPrefix()
{
	return "";
}

// $deBlob: FUNCTION 005faa10
TBOOL APaintParticleSystem::Create()
{
	m_DummyMaterial.Create();

	TASSERT( m_DummyMaterial.m_pShader == TNULL );
	m_DummyMaterial.m_pShader = this;

	m_DummyMesh.Create();

	TASSERT( m_DummyMesh.m_pOwnerShader == TNULL );
	m_DummyMesh.m_pOwnerShader = this;

	m_DummyMesh.Validate();

	m_OrderTable.Create( this, 5340, 0xFF );
	m_OrderTable.RegisterMaterial( &m_DummyMaterial );

	Reset();

	TRenderDX11*  pRender = TRenderDX11::Interface();
	ID3D11Device* pDevice = pRender->GetDevice();

	ID3DBlob* pVSBlob = TRenderDX11::CompileShader( s_szPaintVS, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader );

	D3D11_INPUT_ELEMENT_DESC aInputDescs[ 2 ] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	pDevice->CreateInputLayout( aInputDescs, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );
	pVSBlob->Release();

	ID3DBlob* pPSBlob = TRenderDX11::CompileShader( s_szPaintPS, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader );
	pPSBlob->Release();

	Validate();

	return TShader::Create();
}

// $deBlob: FUNCTION 005fa930
void APaintParticleSystem::Reset()
{
	m_lFreeList.Clear();
	m_lUsedList.Clear();
	m_lDecalList.Clear();

	for ( TINT i = 0; i < MAX_PARTICLES; i++ )
		m_lFreeList.PushBack( &m_aParticles[ i ] );

	// NOTE: 400 cells: 12 bytes of 0xFF each in the two buffers, a zero paint alpha in the grid
	for ( TINT i = 0; i < 400; i++ )
	{
		std::memset( TREINTERPRETCAST( TUINT8*, m_pPaintBufferA ) + i * 12, 0xFF, 12 );
		std::memset( TREINTERPRETCAST( TUINT8*, m_pPaintBufferB ) + i * 12, 0xFF, 12 );
		m_pPaintCells[ i ] = 0;
	}

	m_bDirty      = TFALSE;
	m_fFadeTimer  = 0.0f;
	m_iLineCursor = 0;
	m_iLineCount  = 0;
}

// $deBlob: FUNCTION 005fa6e0
TINT APaintParticleSystem::AddParticle( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rVelocity, TUINT32 a_uiColour, TFLOAT a_fRadius, TINT a_iDecal, TINT a_iParam38, TINT a_iParam40, TINT a_iParam44 )
{
	Particle* pParticle;

	// NOTE: the free particles first, then the used ones (the oldest), then the decals
	if ( !m_lFreeList.IsEmpty() )
		pParticle = m_lFreeList.PopFront();
	else if ( !m_lUsedList.IsEmpty() )
		pParticle = m_lUsedList.PopFront();
	else if ( !m_lDecalList.IsEmpty() )
		pParticle = m_lDecalList.PopFront();
	else
		return -1;

	pParticle->m_vPosition  = a_rPosition;
	pParticle->m_vVelocity  = a_rVelocity;
	pParticle->m_uiColour   = a_uiColour;
	pParticle->m_fRadius    = a_fRadius;
	pParticle->m_fLifeTime  = 2.0f;
	pParticle->m_bDecal     = a_iDecal;
	pParticle->m_iParam38   = a_iParam38;
	pParticle->m_iLineIndex = -1;
	pParticle->m_iParam40   = a_iParam40;
	pParticle->m_iParam44   = a_iParam44;

	if ( a_iDecal == 0 )
		m_lUsedList.PushBack( pParticle );
	else
		m_lDecalList.PushBack( pParticle );

	return TINT( pParticle - m_aParticles );
}

// $deBlob: FUNCTION 005fa820
void APaintParticleSystem::ReleaseParticle( Particle* a_pParticle )
{
	if ( a_pParticle != TNULL )
	{
		TASSERT( m_lUsedList.IsInList( a_pParticle ) || m_lDecalList.IsInList( a_pParticle ) );

		a_pParticle->Remove();
		m_lFreeList.PushBack( a_pParticle );
	}
}

// $deBlob: FUNCTION 005fa8a0
void APaintParticleSystem::AddRenderCall( ACamera* a_pCamera )
{
	if ( !m_bEnabled )
		return;

	Toshi::TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( a_pCamera );
	if ( pViewport == TNULL )
		return;

	Toshi::TRenderPacket* pRenderPacket = m_DummyMaterial.m_pRegMaterial->AddRenderPacket( &m_DummyMesh );
	pRenderPacket->SetModelViewMatrix( pViewport->GetRenderContext()->GetWorldViewMatrix() );

	m_mMatrix = a_pCamera->GetMatrix();
}

// $deBlob: FUNCTION 005fa420
void APaintParticleSystem::FadePaint( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnabled )
		return;

	// The fade: the paint of a cell goes down by a step that depends on the number of lines (above 100 lines the
	// fade slows down)
	TINT iOverflow = m_iLineCount - 100;
	m_fFadeTimer -= a_fDeltaTime;

	TFLOAT fStep = a_fDeltaTime * 255.0f;
	TINT   iAmount;
	if ( iOverflow <= 0 )
		iAmount = TINT( fStep / 0.0625f );
	else
		iAmount = TINT( fStep / TMath::LerpTowards( 0.0625f, 0.001f, TFLOAT( iOverflow ) / 100.0f ) );

	std::memcpy( m_pPaintBufferB, m_pPaintBufferA, 4800 );
	m_iLineCount = 0;

	for ( TINT i = 0; i < 400; i++ )
	{
		TUINT8* pAlpha = TREINTERPRETCAST( TUINT8*, &m_pPaintCells[ i ] ) + 3;
		if ( *pAlpha == 0 )
			continue;

		TINT iNewAlpha = TINT( *pAlpha ) - iAmount;
		if ( iNewAlpha > 0 )
		{
			*pAlpha = TUINT8( iNewAlpha );

			// The odd cells are the lines
			if ( ( i & 1 ) != 0 )
			{
				m_pLineIndices[ m_iLineCount ] = TUINT16( i & 0xFFFE );
				m_iLineCount++;

				TASSERT( m_iLineCount <= MAX_LINES );
			}
		}
		else
		{
			*pAlpha = 0;
		}
	}

	// The particles of the used list, then of the decal list (the next node is read before the update, which can unlink it)
	{
		Particle* pEnd      = m_lUsedList.End();
		Particle* pParticle = m_lUsedList.Begin();
		while ( pParticle != pEnd )
		{
			Particle* pNext = TSTATICCAST( Particle, pParticle->Next() );
			UpdateParticle( pParticle, a_fDeltaTime, a_fDeltaTime );
			pParticle = pNext;
		}
	}

	{
		Particle* pEnd      = m_lDecalList.End();
		Particle* pParticle = m_lDecalList.Begin();
		while ( pParticle != pEnd )
		{
			Particle* pNext = TSTATICCAST( Particle, pParticle->Next() );
			UpdateParticle( pParticle, a_fDeltaTime, a_fDeltaTime );
			pParticle = pNext;
		}
	}

	// Swap the paint buffers
	void* pBufferA  = m_pPaintBufferA;
	m_pPaintBufferA = m_pPaintBufferB;
	m_pPaintBufferB = pBufferA;

	if ( !( m_fFadeTimer >= 0.0f ) )
		m_fFadeTimer = 0.05f;
}

// The vertex of the paint lines (0x10 bytes): the POSITION and COLOR inputs of s_szPaintVS
struct PaintVertex
{
	TFLOAT  m_fX;       // 0x00
	TFLOAT  m_fY;       // 0x04
	TFLOAT  m_fZ;       // 0x08
	TUINT32 m_uiColour; // 0x0C
};

TSTATICASSERT( sizeof( PaintVertex ) == 0x10 );

// A cell position of the paint buffers (0x0C bytes)
struct PaintPosition
{
	TFLOAT m_fX; // 0x00
	TFLOAT m_fY; // 0x04
	TFLOAT m_fZ; // 0x08
};

TSTATICASSERT( sizeof( PaintPosition ) == 0x0C );

// The squared distance of a position from the camera (the translation row of the shake matrix m_mMatrix)
static TFLOAT GetPaintDistanceSqr( const PaintPosition& a_rPosition, const TMatrix44& a_rCamera )
{
	TFLOAT fDX = a_rPosition.m_fX - a_rCamera.m_f41;
	TFLOAT fDY = a_rPosition.m_fY - a_rCamera.m_f42;
	TFLOAT fDZ = a_rPosition.m_fZ - a_rCamera.m_f43;

	return fDX * fDX + fDY * fDY + fDZ * fDZ;
}

// The colour of a paint cell at a squared distance from the camera: the alpha (top byte) falls off linearly between
// 1600.0f and 4600.0f (the constants at 0x97db70 and 0x97db74)
static TUINT32 GetPaintVertexColour( TUINT32 a_uiCell, TFLOAT a_fDistanceSqr )
{
	TFLOAT fDistance = a_fDistanceSqr;
	if ( fDistance > 4600.0f )
		fDistance = 4600.0f;
	if ( fDistance < 1600.0f )
		fDistance = 1600.0f;

	TUINT32 uiAlpha    = a_uiCell >> 24;
	TUINT32 uiNewAlpha = uiAlpha - TUINT32( TINT( ( ( fDistance - 1600.0f ) * TFLOAT( uiAlpha ) ) / ( 4600.0f - 1600.0f ) ) );

	return ( uiNewAlpha << 24 ) | ( a_uiCell & 0x00FFFFFF );
}

static void GetPhantomBox( TPhysicsPhantomContainer* a_pContainer, TVector4* a_pBox );

// The distance beyond the box's radius up to which DebugCullPhantom tests a phantom
static TFLOAT s_fPhantomCullRange = 150.0f;

// A debug text line (0x268 bytes: a text box and its text, like APlayerManager::VistaScoreText)
struct DebugText
{
	// $deBlob: FUNCTION 005aa2b0
	DebugText() {}
	// $deBlob: FUNCTION 005aa2e0
	~DebugText() {}

	T2GUITextBox m_oTextBox;       // 0x000
	TWCHAR       m_wszText[ 256 ]; // 0x068
};

// $deBlob: FUNCTION 0053ba50
// $deBlob: FUNCTION 00857fb0
static DebugText s_oDebugText1;
// $deBlob: FUNCTION 0053ba70
// $deBlob: FUNCTION 00857fc0
static DebugText s_oDebugText2;

// The squared range
// $deBlob: FUNCTION 0053ba90
TMAYBE_UNUSED static TFLOAT s_fPhantomCullRangeSq = s_fPhantomCullRange * s_fPhantomCullRange;

// Culls the box of the phantom against the current camera's view planes when it's within s_fPhantomCullRange of it
// (the result isn't used)
// $deBlob: FUNCTION 005aa100
void DebugCullPhantom( TPhysicsPhantomContainer* a_pContainer, void* a_pUnused )
{
	TVector4 aBox[ 2 ];
	GetPhantomBox( a_pContainer, aBox );

	TFLOAT   fRadius   = aBox[ 1 ].Magnitude();
	ACamera* pCamera   = ACameraManager::GetSingleton()->GetCurrentCamera();
	TFLOAT   fDistance = TVector4::Distance( pCamera->GetMatrix().GetTranslation(), aBox[ 0 ] );

	if ( fDistance <= s_fPhantomCullRange + fRadius )
	{
		TRender::GetSingleton()->GetCurrentContext();
		TSphere oSphere( aBox[ 0 ].AsVector3(), fRadius );
		TRenderContext::CullSphereToFrustumSimple( oSphere, pCamera->GetViewPlanes(), 6 );
	}
}

// A vertex of a custom mesh: a TINT16 vector scaled by 1 / ( 1 << a_iShift), w 1
// $deBlob: FUNCTION 005aa240
void DecompressVertex( TVector4& a_rOutVertex, const TINT16* a_pVertex, TINT8 a_iShift )
{
	a_rOutVertex.x = TFLOAT( a_pVertex[ 0 ] );
	a_rOutVertex.y = TFLOAT( a_pVertex[ 1 ] );
	a_rOutVertex.z = TFLOAT( a_pVertex[ 2 ] );
	a_rOutVertex.Multiply( 1.0f / TFLOAT( 1 << a_iShift ) );
	a_rOutVertex.w = 1.0f;
}

// The box of the static phantom of a particle: the centre (a_pBox[ 0 ]) and the half extents (a_pBox[ 1 ]) of its AABB
// phantom, the zero box (0, 0, 0, 1) without a phantom
// NOTE: retail calls the phantom's virtual at +0x18; this reads hkpAabbPhantom::getAabb (Havok 6.5, not the 5.5 slot)
// $deBlob: FUNCTION 005aa300
static void GetPhantomBox( TPhysicsPhantomContainer* a_pContainer, TVector4* a_pBox )
{
	if ( a_pContainer == TNULL || a_pContainer->m_phantom == TNULL )
	{
		a_pBox[ 0 ] = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		a_pBox[ 1 ] = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		return;
	}

	const hkAabb& rAabb = TSTATICCAST( hkpAabbPhantom, a_pContainer->m_phantom )->getAabb();
	const TFLOAT* pMin  = TREINTERPRETCAST( const TFLOAT*, &rAabb.m_min );
	const TFLOAT* pMax  = TREINTERPRETCAST( const TFLOAT*, &rAabb.m_max );

	a_pBox[ 0 ] = TVector4( ( pMin[ 0 ] + pMax[ 0 ] ) * 0.5f, ( pMin[ 1 ] + pMax[ 1 ] ) * 0.5f, ( pMin[ 2 ] + pMax[ 2 ] ) * 0.5f, 0.5f );
	a_pBox[ 1 ] = TVector4( ( pMax[ 0 ] - pMin[ 0 ] ) * 0.5f, ( pMax[ 1 ] - pMin[ 1 ] ) * 0.5f, ( pMax[ 2 ] - pMin[ 2 ] ) * 0.5f, 0.5f );
}

// Whether a point is inside a box (centre a_pBox[ 0 ], half extents a_pBox[ 1 ]), the bounds included on each axis
// $deBlob: FUNCTION 0068b5c0
static TBOOL IsPointInPhantomBox( const TVector4* a_pBox, const TVector4& a_rPoint )
{
	return a_rPoint.x >= a_pBox[ 0 ].x - a_pBox[ 1 ].x && a_rPoint.y >= a_pBox[ 0 ].y - a_pBox[ 1 ].y && a_rPoint.z >= a_pBox[ 0 ].z - a_pBox[ 1 ].z && a_pBox[ 0 ].x + a_pBox[ 1 ].x >= a_rPoint.x && a_pBox[ 0 ].y + a_pBox[ 1 ].y >= a_rPoint.y && a_pBox[ 0 ].z + a_pBox[ 1 ].z >= a_rPoint.z;
}

// $deBlob: FUNCTION 005fb0c0
void APaintParticleSystem::UpdateParticle( Particle* a_pParticle, TFLOAT a_fDeltaTime, TFLOAT a_fDeltaTime2 )
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	// The position before the update: the start of the trail and of the ray
	TVector4 vOldPosition = a_pParticle->m_vPosition;

	a_pParticle->m_fLifeTime -= a_fDeltaTime;
	if ( a_pParticle->m_fLifeTime < 0.0f )
	{
		ReleaseParticle( a_pParticle );
		return;
	}

	// Move by the velocity, fall by gravity
	a_pParticle->m_vPosition.x += a_pParticle->m_vVelocity.x * a_fDeltaTime2;
	a_pParticle->m_vPosition.y += a_pParticle->m_vVelocity.y * a_fDeltaTime2;
	a_pParticle->m_vPosition.z += a_pParticle->m_vVelocity.z * a_fDeltaTime2;
	a_pParticle->m_vPosition.y = m_fGravity * 0.5f * a_fDeltaTime2 * a_fDeltaTime2 + a_pParticle->m_vPosition.y;

	// The particle leaves the broad phase box of the physics world: it is gone
	const TVector4& rMin = pPhysicsManager->GetInitData().m_vBroadPhaseMin;
	const TVector4& rMax = pPhysicsManager->GetInitData().m_vBroadPhaseMax;
	if ( a_pParticle->m_vPosition.x <= rMin.x || rMax.x <= a_pParticle->m_vPosition.x || a_pParticle->m_vPosition.y <= rMin.y || rMax.y <= a_pParticle->m_vPosition.y || a_pParticle->m_vPosition.z <= rMin.z || rMax.z <= a_pParticle->m_vPosition.z )
	{
		ReleaseParticle( a_pParticle );
		return;
	}

	a_pParticle->m_vVelocity.y = m_fGravity * a_fDeltaTime2 + a_pParticle->m_vVelocity.y;

	if ( m_fFadeTimer > 0.0f || m_fFadeTimer == 0.0f )
	{
		// Not fading: the line of the particle (if it has one) follows it
		if ( a_pParticle->m_iLineIndex >= 0 )
		{
			PaintPosition* pPositions = TREINTERPRETCAST( PaintPosition*, m_pPaintBufferB );

			pPositions[ a_pParticle->m_iLineIndex ].m_fX = a_pParticle->m_vPosition.x;
			pPositions[ a_pParticle->m_iLineIndex ].m_fY = a_pParticle->m_vPosition.y;
			pPositions[ a_pParticle->m_iLineIndex ].m_fZ = a_pParticle->m_vPosition.z;
			m_pPaintCells[ a_pParticle->m_iLineIndex ]   = a_pParticle->m_uiColour;
		}
	}
	else
	{
		// Fading: the old position goes to the ring of lines (cursor), then the new one (cursor + 1), a line from
		// the pair of cells, which gets the colour of the particle
		PaintPosition* pPositions = TREINTERPRETCAST( PaintPosition*, m_pPaintBufferB );

		pPositions[ m_iLineCursor ].m_fX = vOldPosition.x;
		pPositions[ m_iLineCursor ].m_fY = vOldPosition.y;
		pPositions[ m_iLineCursor ].m_fZ = vOldPosition.z;

		if ( a_pParticle->m_iLineIndex < 0 )
			m_pPaintCells[ m_iLineCursor ] = a_pParticle->m_uiColour & 0x00FFFFFF; // a transparent start
		else
			m_pPaintCells[ m_iLineCursor ] = m_pPaintCells[ a_pParticle->m_iLineIndex ];

		m_iLineCursor++;

		TUINT32 uiOldAlpha = m_pPaintCells[ m_iLineCursor ] >> 24;

		pPositions[ m_iLineCursor ].m_fX = a_pParticle->m_vPosition.x;
		pPositions[ m_iLineCursor ].m_fY = a_pParticle->m_vPosition.y;
		pPositions[ m_iLineCursor ].m_fZ = a_pParticle->m_vPosition.z;
		m_pPaintCells[ m_iLineCursor ]   = a_pParticle->m_uiColour;
		a_pParticle->m_iLineIndex        = m_iLineCursor;

		if ( uiOldAlpha == 0 )
		{
			m_pLineIndices[ m_iLineCount ] = TUINT16( m_iLineCursor & 0xFFFE );
			m_iLineCount++;

			TASSERT( m_iLineCount <= MAX_LINES );
		}

		m_iLineCursor++;
		if ( m_iLineCursor > 199 )
			m_iLineCursor = 0;
	}

	// Only the decal particles land on the world
	if ( a_pParticle->m_bDecal == 0 )
		return;

	// The ray of the move, in the collision layer 0xA1
	TPhysicsRay oRay;
	oRay.m_vFrom            = vOldPosition;
	oRay.m_vTo              = a_pParticle->m_vPosition;
	oRay.m_uiCollisionLayer = 0xA1;

	TPhysicsPhantomContainer* pStaticGeo = TREINTERPRETCAST( TPhysicsPhantomContainer*, a_pParticle->m_iParam40 );
	TPhysicsPhantomContainer* pDecalGeo  = TREINTERPRETCAST( TPhysicsPhantomContainer*, a_pParticle->m_iParam44 );

	// Both ends inside the box of the static phantom: the ray goes through it (in the simulation), else the whole world
	TVector4 aBox[ 2 ];
	GetPhantomBox( pStaticGeo, aBox );

	TBOOL bInBox = IsPointInPhantomBox( aBox, oRay.m_vTo ) && IsPointInPhantomBox( aBox, vOldPosition );

	TPhysicsPhantomContainer* pPhantom = TNULL;
	if ( bInBox && TPhysicsManager::IsPhantomInSimulation( pStaticGeo ) )
	{
		TASSERT( pPhysicsManager->GetCollisionTestFlags( pStaticGeo ) == 0xA1 );
		pPhantom = pStaticGeo;
	}

	TPhysicsShapeContact oContact;
	if ( !pPhysicsManager->CollideRay( oRay, oContact, pPhantom ) )
		return;

	if ( ( oContact.m_uiMaterial & 0x10 ) == 0 )
	{
		if ( ( oContact.m_uiMaterial & 0x2000 ) == 0 )
		{
			TBOOL bHit = TTRUE;

			if ( ( TPhysicsManager::GetCollisionLayer( oContact.m_pBody ) & 8 ) == 0 )
			{
				// The decal ray goes through the decal phantom in the game play world
				TINT iOldWorld = pPhysicsManager->GetDefaultWorld();
				pPhysicsManager->SetDefaultWorld( 1 );

				oRay.m_uiCollisionLayer = 8;

				TPhysicsPhantomContainer* pDecalPhantom = TNULL;
				if ( bInBox && TPhysicsManager::IsPhantomInSimulation( pDecalGeo ) )
				{
					TASSERT( pPhysicsManager->GetCollisionTestFlags( pDecalGeo ) == 8 );
					pDecalPhantom = pDecalGeo;
				}

				bHit = pPhysicsManager->CollideRay( oRay, oContact, pDecalPhantom );

				TASSERT( iOldWorld == APhysicsWorldType_GamePlay );
				pPhysicsManager->SetDefaultWorld( iOldWorld );
			}

			if ( !bHit )
				return;

			// The speed scales the size of the decal (0.2 to 1.0)
			TFLOAT fScale = a_pParticle->m_vVelocity.Magnitude();
			fScale        = fScale / 100.0f;
			Toshi::TMath::Clip( fScale, 0.2f, 1.0f );

			// The decal sits a little off the surface, on the separating normal
			// NOTE: the w is the contact's (1.0 for a point, ADecalManager asserts it)
			TVector4 vDecalPosition = oContact.m_vPosition;
			vDecalPosition.x        = oContact.m_vPosition.x + oContact.m_vNormal.x * 0.1f;
			vDecalPosition.y        = oContact.m_vPosition.y + oContact.m_vNormal.y * 0.1f;
			vDecalPosition.z        = oContact.m_vPosition.z + oContact.m_vNormal.z * 0.1f;

			if ( a_pParticle->m_bDecal != 0 )
			{
				TFLOAT fRandom = ARandom::GetSingleton()->GetRandom().GetFloat();
				ADecalManager::GetSingleton()->SetRotation( fRandom * 6.2831855f );

				TFLOAT fSize = a_pParticle->m_fRadius * fScale;
				ADecalManager::GetSingleton()->AddDecal(
				    &vDecalPosition,
				    fSize,
				    fSize,
				    a_pParticle->m_uiColour,
				    TREINTERPRETCAST( Toshi::TMaterial*, a_pParticle->m_bDecal ),
				    ( fRandom <= 0.5f ) ? ADecalManager::Type_BottomRight : ADecalManager::Type_TopRight
				);
			}
		}
	}
	else if ( a_pParticle->m_iParam38 != 0 )
	{
		// An effect at the contact: its transform is put at the contact and scaled down
		TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( TREINTERPRETCAST( TParticleEffect*, a_pParticle->m_iParam38 ) );
		if ( pEffect != TNULL )
		{
			TASSERT( oContact.m_vPosition.w == 1.0f );

			TMatrix44& rTransform = pEffect->GetTransform();
			rTransform.m_f41      = oContact.m_vPosition.x;
			rTransform.m_f42      = oContact.m_vPosition.y;
			rTransform.m_f43      = oContact.m_vPosition.z;
			rTransform.m_f44      = oContact.m_vPosition.w;
			rTransform.Scale( 0.1f, 0.1f, 0.1f );

			ReleaseParticle( a_pParticle );
			return;
		}
	}

	ReleaseParticle( a_pParticle );
}


// $deBlob: FUNCTION 005fac80
void APaintParticleSystem::Render( Toshi::TRenderPacket* a_pRenderPacket )
{
	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	// The world view projection of the packet: ModelView * Projection (XMMatrixMultiply, the same product as TPrimShader)
	TRenderContextDX11* pRenderContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
	TMatrix44           oProjection    = pRenderContext->GetProjectionMatrix();
	TMatrix44           oModelView     = a_pRenderPacket->GetModelViewMatrix();

	TMatrix44 oWorldViewProjection;
	DirectX::XMStoreFloat4x4(
	    TREINTERPRETCAST( DirectX::XMFLOAT4X4*, &oWorldViewProjection ),
	    DirectX::XMMatrixMultiply(
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oModelView ) ),
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oProjection ) )
	    )
	);
	pRender->SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );

	pRender->SetDstAlpha( -1.0f );
	pRender->SetAlphaUpdate( TFALSE );
	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

	// The current rasterizer state: its low byte becomes (byte & 0xF3) | 4 (bit 3 off, bit 2 on)
	TRenderDX11::RASTERIZERSTATE& rRasterizer = pRender->GetCurrentRasterizer();
	rRasterizer.uiFlags                       = ( rRasterizer.uiFlags & ~0x0CU ) | 0x04U;

	// Two vertices per line: the cell and the cell after it (the odd one of the pair), read from the buffer A
	D3D11_MAPPED_SUBRESOURCE oMapped;
	pDeviceContext->Map( m_pVertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &oMapped );

	PaintVertex*         pVertices  = TREINTERPRETCAST( PaintVertex*, oMapped.pData );
	const PaintPosition* pPositions = TREINTERPRETCAST( const PaintPosition*, m_pPaintBufferA );

	for ( TINT i = 0; i < m_iLineCount; i++ )
	{
		TUINT16 uiCell = m_pLineIndices[ i ];

		const PaintPosition& rPositionA = pPositions[ uiCell ];
		const PaintPosition& rPositionB = pPositions[ uiCell + 1 ];
		TUINT32              uiCellA    = m_pPaintCells[ uiCell ];
		TUINT32              uiCellB    = m_pPaintCells[ uiCell + 1 ];

		PaintVertex* pVertexA = &pVertices[ i * 2 ];
		PaintVertex* pVertexB = &pVertices[ i * 2 + 1 ];

		pVertexA->m_fX       = rPositionA.m_fX;
		pVertexA->m_fY       = rPositionA.m_fY;
		pVertexA->m_fZ       = rPositionA.m_fZ;
		pVertexA->m_uiColour = GetPaintVertexColour( uiCellA, GetPaintDistanceSqr( rPositionA, m_mMatrix ) );

		pVertexB->m_fX       = rPositionB.m_fX;
		pVertexB->m_fY       = rPositionB.m_fY;
		pVertexB->m_fZ       = rPositionB.m_fZ;
		pVertexB->m_uiColour = GetPaintVertexColour( uiCellB, GetPaintDistanceSqr( rPositionB, m_mMatrix ) );
	}

	TUINT32 uiVertexCount = TUINT32( m_iLineCount ) * 2;

	pDeviceContext->Unmap( m_pVertexBuffer, 0 );

	pDeviceContext->IASetInputLayout( m_pInputLayout );
	pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader, TNULL, 0 );

	pRender->DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY_LINELIST, m_pVertexBuffer, uiVertexCount, sizeof( PaintVertex ), 0, 0 );
}
