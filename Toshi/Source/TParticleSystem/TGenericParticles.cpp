#include "ToshiPCH.h"
#include "TGenericParticles.h"
#include "TGenericParticlesSettings.h"
#include "TParticleSystem.h"
#include "TParticleMesh.h"
#include "TParticleMaterial.h"
#include "TParticleAffector.h"
#include "TParticleKeyframe.h"
#include "Render/TRender.h"
#include "Render/TRenderContext.h"
#include "Render/TModel.h"
#include "Toshi/TSystem.h"
#include "Toshi/TScheduler.h"
#include "Math/TRandom.h"
#include "Math/TMathInline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TVector4 TGenericParticles::s_vBLOffset;
TVector4 TGenericParticles::s_vTLOffset;
TVector4 TGenericParticles::s_vBROffset;
TVector4 TGenericParticles::s_vTROffset;
TVector4 TGenericParticles::s_avQuadRotations[ MAX_ROTATIONS * 4 ];
TINT     TGenericParticles::s_iNumRotations     = 1;
TINT     TGenericParticles::s_iLightR           = 255;
TINT     TGenericParticles::s_iLightG           = 255;
TINT     TGenericParticles::s_iLightB           = 255;
TUINT32  TGenericParticles::s_uiConstCol0       = 0;
TUINT32  TGenericParticles::s_uiConstCol1       = 0;
TFLOAT   TGenericParticles::s_fConstSize0       = 0.0f;
TFLOAT   TGenericParticles::s_fConstSize1       = 0.0f;
TFLOAT   TGenericParticles::s_fYoungestLife     = 0.0f;
TFLOAT   TGenericParticles::s_fOldestLife       = 1.0f;
TFLOAT   TGenericParticles::s_fRibbonOldestLife = 0.0f;
TVector4 TGenericParticles::s_vRibbonUp;

// $deBlob: FUNCTION 0053fe90
static TRandom s_oRandom;

// $deBlob: FUNCTION 00718830
TMAYBE_UNUSED static TINT IntLog2Ceil( TINT a_iValue )
{
	TFLOAT fValue = TFLOAT( a_iValue * 2 - 1 );
	return ( *TREINTERPRETCAST( TINT*, &fValue ) >> 23 ) - 0x7F;
}

// $deBlob: FUNCTION 007189c0
static TINT NextPowerOf2( TINT a_iValue )
{
	TFLOAT fValue = TFLOAT( a_iValue * 2 - 1 );
	return 1 << ( ( *TREINTERPRETCAST( TINT*, &fValue ) >> 23 ) - 0x7F );
}

// TMath::IntLog2 with the float's exponent
static TINT FloatIntLog2( TINT a_iValue )
{
	TFLOAT fValue = TFLOAT( a_iValue );
	return ( *TREINTERPRETCAST( TINT*, &fValue ) >> 23 ) - 0x7F;
}

static TFLOAT GetSchedulerDeltaTime()
{
	return TSystemManager::GetSingleton()->GetScheduler()->GetCurrentDeltaTime();
}

// $deBlob: FUNCTION 00712d60
TGenericParticles::TGenericParticles()
{
	m_fnFinished        = TNULL;
	m_pFinishedUserData = TNULL;
	m_uiFlags           = 0;
	m_iNumEmitting      = 0;
	m_iNumEmitted       = 0;
	m_pSettings         = TNULL;
	m_oTransform.Identity();
}

// $deBlob: FUNCTION 00712e30
// $deBlob: FUNCTION 007183d0
// $deBlob: FUNCTION 0070ec30
TGenericParticles::~TGenericParticles()
{
}

// $deBlob: FUNCTION 00712fb0
void TGenericParticles::SetSettings( TGenericParticlesSettings* a_pSettings )
{
	SetNumParticles( 0 );

	m_pSettings         = a_pSettings;
	m_uiFlags           = a_pSettings->m_uiBuffers;
	m_pMaterial         = a_pSettings->m_pMaterial;
	m_fLODDistanceScale = 1.0f / ( a_pSettings->m_fLODDistance * a_pSettings->m_fLODDistance );
	m_iMinNumParticles  = a_pSettings->m_iMinParticles;

	if ( !a_pSettings->m_bDynamicRadius )
		m_oBoundingSphere.GetRadius() = a_pSettings->m_fRadius;
}

// $deBlob: FUNCTION 00713010
void TGenericParticles::Initialise()
{
	m_iNumRenderingParticles = 0;
	m_iNumEmitting           = 0;
	m_iNumEmitted            = 0;
	m_fTime                  = 0.0f;

	TMatrix44::MatToQuat( m_qPrevRotation, m_oTransform );
	m_vPrevTranslation = m_oTransform.GetTranslation();

	if ( m_pSettings->m_uiFlags & TGenericParticlesSettings::FLAGS_ROTATIONS )
		InitRotations();

	TGenericParticlesSettings* pSettings = m_pSettings;

	if ( pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_MESH )
	{
		m_fnSetPosition = TNULL;
		m_fnSetUV       = TNULL;
		m_fnSetColour   = TNULL;
		m_fnSetIndices  = TNULL;
		m_fnGetSize     = TNULL;
	}
	else
	{
		m_fnSetIndices = TParticles::SetQuadIndices;

		TGenericParticlesSettings::RENDERSTYLE eStyle = pSettings->m_eRenderStyle;

		if ( eStyle == TGenericParticlesSettings::RENDERSTYLE_QUAD )
		{
			m_fnSetPosition = ( pSettings->m_uiFlags & TGenericParticlesSettings::FLAGS_ROTATIONS ) ? SetPositionSizeBlendRotate : SetPositionSizeBlend;
		}
		else if ( eStyle == TGenericParticlesSettings::RENDERSTYLE_BILLBOARDQUAD )
		{
			m_fnSetPosition = SetPositionSizeBlendRotate;
		}
		else if ( eStyle == TGenericParticlesSettings::RENDERSTYLE_RIBBON )
		{
			m_fnSetPosition = TNULL;
			m_fnSetIndices  = RenderRibbonIndices;
		}
		else if ( eStyle == TGenericParticlesSettings::RENDERSTYLE_SPARKQUAD )
		{
			m_fnSetPosition = SetPositionSparkQuad;
		}
		else
		{
			TASSERT( m_pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_SPARKS );
			m_fnSetPosition = SetPositionSpark;
		}

		pSettings = m_pSettings;

		t_SetVertsFn fnColour;
		t_SetVertsFn fnColourLit;

		if ( pSettings->m_pColourKeys == TNULL )
		{
			fnColour    = SetColourFromBuffer;
			fnColourLit = SetColourFromBufferWithLighting;
		}
		else
		{
			fnColour    = SetColourFromKeyframe;
			fnColourLit = SetColourFromKeyframeWithLighting;
		}

		if ( pSettings->m_uiFlags & TGenericParticlesSettings::FLAGS_LIGHTING )
			fnColour = fnColourLit;

		m_fnSetColour = fnColour;
		eStyle        = pSettings->m_eRenderStyle;

		if ( pSettings->m_pUVKeys == TNULL && eStyle == TGenericParticlesSettings::RENDERSTYLE_RIBBON )
		{
			m_fnSetUV = SetConstantUVRibbon;
		}
		else if ( pSettings->m_pUVKeys != TNULL && eStyle == TGenericParticlesSettings::RENDERSTYLE_RIBBON )
		{
			m_fnSetUV = SetUVFromUVIndexRibbon;
		}
		else if ( eStyle == TGenericParticlesSettings::RENDERSTYLE_SPARKS )
		{
			m_fnSetUV = SetUVFromUVIndexSparks;
		}
		else
		{
			m_fnSetUV = ( pSettings->m_uiFlags & TGenericParticlesSettings::FLAGS_UVROTATE ) ? SetUVFromUVIndexRotate : SetUVFromUVIndex;
		}
	}

	if ( pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_BILLBOARDQUAD )
	{
		TBillboardQuadRenderSettings* pBillboard = pSettings->m_pBillboardSettings;

		if ( !HasFlag( FLAGS_TRANSROTWITHXFORM ) && ( pBillboard->m_uiFlags & TBillboardQuadRenderSettings::Flags_EmitterSpace ) )
		{
			// The vectors are in the emitter's space
			TVector3 vUp      = pBillboard->m_vUp;
			TVector3 vForward = pBillboard->m_vForward;
			TMatrix44::RotateVector( vUp, m_oTransform, vUp );
			TMatrix44::RotateVector( vForward, m_oTransform, vForward );
			m_vBillboardUp      = vUp;
			m_vBillboardForward = vForward;
		}
		else
		{
			m_vBillboardUp      = pBillboard->m_vUp;
			m_vBillboardForward = pBillboard->m_vForward;
		}
	}

	m_fnGetSize = ( m_pSettings->m_pSizeKeys == TNULL ) ? GetSizeFixed : GetSizeKeyframe;

	TParticleKeyframeTimeline::InitState( m_aStates[ 0 ] );
	TParticleKeyframeTimeline::InitState( m_aStates[ 1 ] );
	TParticleKeyframeTimeline::InitState( m_aStates[ 2 ] );
	TParticleKeyframeTimeline::InitState( m_aStates[ 3 ] );
}

// $deBlob: FUNCTION 00713350
void TGenericParticles::Deinitialise()
{
	SetNumParticles( 0 );

	m_avPosition.SetSize( 0 );
	m_avVelocity.SetSize( 0 );
	m_avForward.SetSize( 0 );
	m_afLife.SetSize( 0 );
	m_afLifeLostPerSec.SetSize( 0 );
	m_afMass.SetSize( 0 );
	m_afSize.SetSize( 0 );
	m_aiUVIndex.SetSize( 0 );
	m_auiColour.SetSize( 0 );
	m_apCurColourKey.SetSize( 0 );
	m_apCurSizeKey.SetSize( 0 );
	m_auiEventIndex.SetSize( 0 );

	m_uiFlags   = 0;
	m_pSettings = TNULL;
	m_pMaterial = TNULL;

	TParticles::Deinitialise();

	TASSERT( m_avPosition.GetSize() == 0 );
	TASSERT( m_avVelocity.GetSize() == 0 );
	TASSERT( m_afLife.GetSize() == 0 );
	TASSERT( m_afLifeLostPerSec.GetSize() == 0 );
	TASSERT( m_afMass.GetSize() == 0 );
	TASSERT( m_afSize.GetSize() == 0 );
	TASSERT( m_aiUVIndex.GetSize() == 0 );
	TASSERT( m_auiColour.GetSize() == 0 );

	if ( m_fnFinished != TNULL )
	{
		m_fnFinished( this, m_pFinishedUserData );
		m_fnFinished        = TNULL;
		m_pFinishedUserData = TNULL;
	}
}

// $deBlob: FUNCTION 00713b00
void TGenericParticles::SetNumParticles( TINT a_iNumParticles )
{
	TINT iOldNumParticles = m_iNumParticles;

	if ( iOldNumParticles == ( a_iNumParticles & 0xFFFF ) )
		return;

	TParticles::SetNumParticles( a_iNumParticles );

	if ( HasFlag( FLAGS_POSITION ) && !m_avPosition.SetSize( m_iNumParticles ) )
		m_iNumParticles = TMath::Int32ToUInt16( m_avPosition.GetSize() );

	if ( HasFlag( FLAGS_VELOCITY ) && !m_avVelocity.SetSize( m_iNumParticles ) )
		m_iNumParticles = TMath::Int32ToUInt16( m_avVelocity.GetSize() );

	if ( HasFlag( FLAGS_LIFE ) )
	{
		if ( !m_afLife.SetSize( m_iNumParticles ) )
			m_iNumParticles = TMath::Int32ToUInt16( m_afLife.GetSize() );

		if ( !m_afLifeLostPerSec.SetSize( m_iNumParticles ) )
			m_iNumParticles = TMath::Int32ToUInt16( m_afLifeLostPerSec.GetSize() );
	}

	if ( HasFlag( FLAGS_MASS ) && !m_afMass.SetSize( m_iNumParticles ) )
		m_iNumParticles = TMath::Int32ToUInt16( m_afMass.GetSize() );

	if ( HasFlag( FLAGS_SIZE ) )
	{
		if ( !m_afSize.SetSize( m_iNumParticles ) )
			m_iNumParticles = TMath::Int32ToUInt16( m_afSize.GetSize() );

		if ( m_pSettings != TNULL && m_pSettings->m_pSizeKeys != TNULL && !m_apCurSizeKey.SetSize( m_iNumParticles ) )
			m_iNumParticles = TMath::Int32ToUInt16( m_apCurSizeKey.GetSize() );
	}

	if ( HasFlag( FLAGS_UVINDEX ) && !m_aiUVIndex.SetSize( m_iNumParticles ) )
		m_iNumParticles = TMath::Int32ToUInt16( m_aiUVIndex.GetSize() );

	if ( HasFlag( FLAGS_COLOUR ) )
	{
		if ( !m_auiColour.SetSize( m_iNumParticles ) )
			m_iNumParticles = TMath::Int32ToUInt16( m_auiColour.GetSize() );

		if ( m_pSettings != TNULL && m_pSettings->m_pColourKeys != TNULL && !m_apCurColourKey.SetSize( m_iNumParticles ) )
			m_iNumParticles = TMath::Int32ToUInt16( m_apCurColourKey.GetSize() );
	}

	if ( HasFlag( FLAGS_FORWARD ) && !m_avForward.SetSize( m_iNumParticles ) )
		m_iNumParticles = TMath::Int32ToUInt16( m_avForward.GetSize() );

	if ( m_iNumParticles > 0 && m_pSettings->m_pEventKeys != TNULL && !m_auiEventIndex.SetSize( m_iNumParticles ) )
		m_iNumParticles = TMath::Int32ToUInt16( m_auiEventIndex.GetSize() );

	TINT iNumParticles = m_iNumParticles;

	if ( m_iNumRenderingParticles > iNumParticles )
		m_iNumRenderingParticles = iNumParticles;

	InitParticles( iOldNumParticles, iNumParticles, GetSchedulerDeltaTime(), TTRUE, -1.0f );
	m_iNumRenderingParticles = m_iNumParticles;
}

// $deBlob: FUNCTION 00713df0
void TGenericParticles::SetNumRenderingParticles( TINT a_iNumParticles )
{
	TASSERT( a_iNumParticles <= m_iNumParticles );
	TParticles::SetNumRenderingParticles( a_iNumParticles ); // inlined

	m_iNumEmitting = TMath::Max( m_iNumEmitting, m_iNumRenderingParticles );

	if ( m_iNumEmitted < m_iNumEmitting )
		InitParticles( m_iNumEmitted, m_iNumEmitting, 0.0f, TFALSE, -1.0f );
}

// $deBlob: FUNCTION 00713eb0
TINT TGenericParticles::GetVertsPerNumParticles( TINT a_iNumParticles )
{
	if ( m_pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_RIBBON )
		return a_iNumParticles * 2;

	return a_iNumParticles * 4;
}

// $deBlob: FUNCTION 00713ed0
void TGenericParticles::SetTransform( const TMatrix44& a_rTransform )
{
	m_oTransform = a_rTransform;

	if ( !m_pSettings->m_bAveragePosition )
		m_oBoundingSphere.GetOrigin() = m_oTransform.GetTranslation3();
}

// $deBlob: FUNCTION 00713630
void TGenericParticles::PreUpdate( TFLOAT a_fDeltaTime )
{
	TParticles::SetNumRenderingParticles( m_iNumEmitting ); // inlined
	m_iNumEmitted  = m_iNumEmitting;
	m_iNumEmitting = 0;
}

// $deBlob: FUNCTION 00713690
TBOOL TGenericParticles::Update( TFLOAT a_fDeltaTime )
{
	TBOOL bDead = TTRUE;
	m_fTime += a_fDeltaTime;

	if ( m_pSettings->m_pColourKeys ) m_pSettings->m_pColourKeys->UpdateParticles( this, 0, m_iNumRenderingParticles );
	if ( m_pSettings->m_pUVKeys ) m_pSettings->m_pUVKeys->UpdateParticles( this, 0, m_iNumRenderingParticles );
	if ( m_pSettings->m_pMassKeys ) m_pSettings->m_pMassKeys->UpdateParticles( this, 0, m_iNumRenderingParticles );
	if ( m_pSettings->m_pSizeKeys ) m_pSettings->m_pSizeKeys->UpdateParticles( this, 0, m_iNumRenderingParticles );
	if ( m_pSettings->m_pEventKeys ) m_pSettings->m_pEventKeys->UpdateEvents( this, 0, m_iNumRenderingParticles );

	for ( TParticleAffector* pAffector = m_pSettings->m_pAffectors; pAffector != TNULL; pAffector = pAffector->m_pNext )
		pAffector->Update( this, 0, m_iNumRenderingParticles, a_fDeltaTime );

	TINT iNumParticles = HasFlag( FLAGS_REEMIT ) ? m_iNumParticles : m_iNumRenderingParticles;

	for ( TINT i = 0; i < iNumParticles; i++ )
		m_afLife[ i ] = a_fDeltaTime * m_afLifeLostPerSec[ i ] + m_afLife[ i ];

	for ( TINT i = 0; i < iNumParticles; i++ )
	{
		if ( m_afLife[ i ] <= 1.0f )
		{
			bDead = TFALSE;
		}
		else if ( HasFlag( FLAGS_REEMIT ) )
		{
			// Emitted again, with the time it's been dead
			TFLOAT fOverTime = m_afLife[ i ] - 1.0f;
			InitParticles( i, i + 1, fOverTime / m_afLifeLostPerSec[ i ], TTRUE, -1.0f );
			bDead = TFALSE;
		}
	}

	if ( !HasFlag( FLAGS_REEMIT ) )
	{
		for ( TINT i = iNumParticles; i < m_iNumParticles; i++ )
		{
			m_afLife[ i ] = m_afLifeLostPerSec[ i ] * a_fDeltaTime + m_afLife[ i ];

			if ( m_afLife[ i ] <= 1.0f )
				bDead = TFALSE;
		}
	}

	TBOOL bFinished = bDead;

	if ( HasFlag( FLAGS_KEEPALIVE ) )
		bFinished = TFALSE;

	if ( m_iNumRenderingParticles > 0 )
	{
		if ( m_pSettings->m_bAveragePosition == TTRUE )
		{
			TFLOAT fX = 0.0f;
			TFLOAT fY = 0.0f;
			TFLOAT fZ = 0.0f;

			for ( TINT i = 0; i < m_iNumRenderingParticles; i++ )
			{
				const TVector4& rPosition = m_avPosition[ i ];
				fX                        = rPosition.x + fX;
				fY                        = rPosition.y + fY;
				fZ                        = rPosition.z + fZ;
			}

			TFLOAT fScale                 = 1.0f / TFLOAT( m_iNumRenderingParticles );
			m_oBoundingSphere.GetOrigin() = TVector3( fX * fScale, fY * fScale, fZ * fScale );
		}

		if ( m_pSettings->m_bDynamicRadius == TTRUE )
		{
			TVector4 vMin( 0.0f, 0.0f, 0.0f, 0.0f );
			TVector4 vMax( 0.0f, 0.0f, 0.0f, 0.0f );
			TVector4 vDiff;

			for ( TINT i = 0; i < m_iNumRenderingParticles; i++ )
			{
				const TVector4& rPosition = m_avPosition[ i ];
				const TVector3& rCentre   = m_oBoundingSphere.GetOrigin();
				vDiff.x                   = rPosition.x - rCentre.x;
				vDiff.y                   = rPosition.y - rCentre.y;
				vDiff.z                   = rPosition.z - rCentre.z;
				vDiff.w                   = rPosition.w;

				if ( vMin.x <= vDiff.x ) { if ( vMax.x < vDiff.x ) vMax.x = vDiff.x; }
				else vMin.x = vDiff.x;

				if ( vMin.y <= vDiff.y ) { if ( vMax.y < vDiff.y ) vMax.y = vDiff.y; }
				else vMin.y = vDiff.y;

				if ( vMin.z <= vDiff.z ) { if ( vMax.z < vDiff.z ) vMax.z = vDiff.z; }
				else vMin.z = vDiff.z;
			}

			vMin.Abs4();
			vMax.Abs4();

			vDiff.x = ( vMin.x <= vMax.x ) ? vMax.x : vMin.x;
			vDiff.y = ( vMin.y <= vMax.y ) ? vMax.y : vMin.y;
			vDiff.z = ( vMin.z <= vMax.z ) ? vMax.z : vMin.z;

			m_oBoundingSphere.GetRadius() = vDiff.Magnitude();
		}
	}

	TMatrix44::MatToQuat( m_qPrevRotation, m_oTransform );
	m_vPrevTranslation = m_oTransform.GetTranslation();

	return !bFinished;
}

// $deBlob: FUNCTION 00713f30
void TGenericParticles::Render()
{
	TParticleSystem* pSystem    = TParticleSystem::GetSingleton();
	TMatrix44        oModelView = pSystem->GetModelView();

	if ( HasFlag( FLAGS_TRANSROTWITHXFORM ) )
	{
		TMatrix44 oMatrix;
		oMatrix.Multiply( oModelView, m_oTransform );
		pSystem->SetModelView( oMatrix );
	}
	else if ( HasFlag( FLAGS_TRANSWITHXFORM ) )
	{
		TMatrix44 oMatrix = oModelView;
		TMatrix44::TransformVector( oMatrix.GetTranslation(), oMatrix, m_oTransform.GetTranslation() );
		pSystem->SetModelView( oMatrix );
	}

	if ( m_pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_QUAD )
	{
		if ( m_pSettings->m_pQuadSettings->m_bFacing == 0 )
		{
			TParticleSystem* pCreating = TParticleSystem::s_pCreatingSystem;
			s_vBLOffset                = pCreating->m_aOffsets[ 0 ];
			s_vTLOffset                = pCreating->m_aOffsets[ 2 ];
			s_vBROffset                = pCreating->m_aOffsets[ 1 ];
			s_vTROffset                = pCreating->m_aOffsets[ 3 ];
		}
		else
		{
			s_vBLOffset = TVector4( -0.5f, 0.0f, -0.5f, 1.0f );
			s_vTLOffset = TVector4( -0.5f, 0.0f, 0.5f, 1.0f );
			s_vBROffset = TVector4( 0.5f, 0.0f, -0.5f, 1.0f );
			s_vTROffset = TVector4( 0.5f, 0.0f, 0.5f, 1.0f );
		}
	}

	TFLOAT fDeltaTime = 0.0f;
	s_uiConstCol0     = m_pSettings->m_pConstColour->m_uiStart;
	s_uiConstCol1     = m_pSettings->m_pConstColour->m_uiEnd;
	s_fConstSize0     = *TREINTERPRETCAST( TFLOAT*, &m_pSettings->m_pConstSize->m_uiStart );
	s_fConstSize1     = *TREINTERPRETCAST( TFLOAT*, &m_pSettings->m_pConstSize->m_uiEnd );

	m_afConstantUVs[ 0 ] = 0.0f;
	m_afConstantUVs[ 1 ] = 0.0f;
	m_afConstantUVs[ 2 ] = 0.0f;
	m_afConstantUVs[ 3 ] = 1.0f;
	m_afConstantUVs[ 4 ] = 1.0f;
	m_afConstantUVs[ 5 ] = 0.0f;
	m_afConstantUVs[ 6 ] = 1.0f;
	m_afConstantUVs[ 7 ] = 1.0f;

	if ( m_pSettings->m_uiFlags & TGenericParticlesSettings::FLAGS_ROTATIONS )
	{
		if ( pSystem->m_bUpdated )
			fDeltaTime = GetSchedulerDeltaTime();

		UpdateRotations( fDeltaTime );
	}

	if ( m_pSettings->m_uiFlags & TGenericParticlesSettings::FLAGS_LIGHTING )
		UpdateLighting();

	TGenericParticlesSettings::RENDERSTYLE eStyle = m_pSettings->m_eRenderStyle;

	if ( eStyle == TGenericParticlesSettings::RENDERSTYLE_RIBBON )
		RenderVertsRibbon( m_fnSetPosition, m_fnSetUV, m_fnSetColour, m_fnSetIndices, this );
	else if ( eStyle == TGenericParticlesSettings::RENDERSTYLE_MESH )
		RenderVertsMesh();
	else
		RenderVerts( m_fnSetPosition, m_fnSetUV, m_fnSetColour, m_fnSetIndices, this );

	if ( m_uiFlags & ( FLAGS_TRANSWITHXFORM | FLAGS_TRANSROTWITHXFORM ) )
		pSystem->SetModelView( oModelView );
}

// $deBlob: FUNCTION 007141d0
void TGenericParticles::InitRotations()
{
	TINT iNumRotations;

	if ( m_pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_BILLBOARDQUAD )
	{
		iNumRotations = m_pSettings->m_pBillboardSettings->m_iNumRotations;
	}
	else
	{
		TASSERT( m_pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_QUAD );
		iNumRotations = m_pSettings->m_pQuadSettings->m_iNumRotations;
	}

	for ( TINT i = 0; i < iNumRotations; i++ )
	{
		TASSERT( i < 16 );
		m_afRotations[ i ] = s_oRandom.GetFloat2() * TMath::PI;
	}
}

// Sets the 4 corners of the rotation a_iRotation (v0 = -a - b... in the order SetPositionSizeBlendRotate reads them)
static void SetQuadRotation( TINT a_iRotation, const TVector4& a_rA, const TVector4& a_rB )
{
	TVector4* pCorners = &TGenericParticles::s_avQuadRotations[ a_iRotation * 4 ];

	pCorners[ 0 ].x = a_rB.x - a_rA.x;
	pCorners[ 0 ].w = a_rB.w;
	pCorners[ 0 ].y = a_rB.y - a_rA.y;
	pCorners[ 0 ].z = a_rB.z - a_rA.z;
	pCorners[ 1 ].w = a_rB.w;
	pCorners[ 1 ].x = a_rA.x + a_rB.x;
	pCorners[ 1 ].y = a_rA.y + a_rB.y;
	pCorners[ 1 ].z = a_rA.z + a_rB.z;
	pCorners[ 3 ].x = a_rA.x - a_rB.x;
	pCorners[ 3 ].y = a_rA.y - a_rB.y;
	pCorners[ 3 ].z = a_rA.z - a_rB.z;
	pCorners[ 3 ].w = a_rA.w;

	TVector4 vNegB( -a_rB.x, -a_rB.y, -a_rB.z, -a_rB.w );
	pCorners[ 2 ].w = vNegB.w;
	pCorners[ 2 ].x = vNegB.x - a_rA.x;
	pCorners[ 2 ].y = vNegB.y - a_rA.y;
	pCorners[ 2 ].z = vNegB.z - a_rA.z;
}

// $deBlob: FUNCTION 007142e0
void TGenericParticles::UpdateRotations( TFLOAT a_fDeltaTime )
{
	const TMatrix44* pViewModel = &TRender::GetSingleton()->GetCurrentContext()->GetViewModelMatrix();
	TINT             iNumRotations;

	if ( m_pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_BILLBOARDQUAD )
	{
		TBillboardQuadRenderSettings* pBillboard = m_pSettings->m_pBillboardSettings;

		s_iNumRotations = TMath::Min( TINT( pBillboard->m_iNumRotations ), m_iNumRenderingParticles );

		TBOOL bLocal = pBillboard->m_uiFlags & 1;

		TVector4 vUp( m_vBillboardUp.x, m_vBillboardUp.y, m_vBillboardUp.z, 1.0f );
		TVector4 vForward( m_vBillboardForward.x, m_vBillboardForward.y, m_vBillboardForward.z, 1.0f );

		// The basis of the billboards (up, side, forward)
		TMatrix44 oBasis;
		oBasis.m_f11 = vUp.x;
		oBasis.m_f12 = vUp.y;
		oBasis.m_f13 = vUp.z;
		oBasis.m_f14 = 1.0f;
		oBasis.AsBasisVector4( BASISVECTOR_UP ) = TMatrix44::IDENTITY.AsBasisVector4( BASISVECTOR_UP );
		oBasis.AsBasisVector4( BASISVECTOR_UP ).CrossProduct( vUp, vForward );
		oBasis.AsBasisVector4( BASISVECTOR_FORWARD ) = vForward;
		oBasis.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TMatrix44::IDENTITY.AsBasisVector4( BASISVECTOR_TRANSLATION );

		if ( bLocal )
		{
			vUp      = TVector4( 1.0f, 0.0f, 0.0f, 1.0f );
			vForward = TVector4( 0.0f, 0.0f, 1.0f, 1.0f );
		}

		iNumRotations = s_iNumRotations;

		for ( TINT i = 0; i < iNumRotations; i++ )
		{
			TASSERT( i < 16 );

			TFLOAT fTime       = m_afRotations[ i ] + a_fDeltaTime;
			m_afRotations[ i ] = fTime;

			TFLOAT fAngleY = pBillboard->m_afRotationsY[ i ] * fTime;
			TFLOAT fAngleX = pBillboard->m_afRotationsX[ i ] * fTime;
			TFLOAT fAngleZ = pBillboard->m_afRotationsZ[ i ] * fTime;

			TVector4 vA = vUp;
			TVector4 vB = vForward;
			vA.RotateX( fAngleX );
			vA.RotateY( fAngleY );
			vA.RotateZ( fAngleZ );
			vB.RotateX( fAngleX );
			vB.RotateY( fAngleY );
			vB.RotateZ( fAngleZ );

			if ( bLocal )
			{
				TMatrix44::RotateVector( vB, oBasis, vB );
				TMatrix44::RotateVector( vA, oBasis, vA );
			}

			SetQuadRotation( i, vA, vB );
			iNumRotations = s_iNumRotations;
		}
	}
	else
	{
		TASSERT( m_pSettings->m_eRenderStyle == TGenericParticlesSettings::RENDERSTYLE_QUAD );

		TQuadRenderSettings* pQuad = m_pSettings->m_pQuadSettings;
		iNumRotations              = TMath::Min( pQuad->m_iNumRotations, m_iNumRenderingParticles );
		s_iNumRotations            = iNumRotations;

		if ( pQuad->m_bFacing == 0 )
		{
			for ( TINT i = 0; i < iNumRotations; i++ )
			{
				TASSERT( i < 16 );

				TFLOAT fAngle      = pQuad->m_afRotations[ i ] * a_fDeltaTime + m_afRotations[ i ];
				m_afRotations[ i ] = fAngle;

				TFLOAT fSin, fCos;
				TMath::SinCos( fAngle, fSin, fCos );

				// Rotated in the view plane
				TVector4 vA( fCos * 0.5f, fSin * 0.5f, 0.0f, 1.0f );
				TVector4 vB( -( fSin * 0.5f ), fCos * 0.5f, 0.0f, 1.0f );
				TMatrix44::RotateVector( vA, *pViewModel, vA );
				TMatrix44::RotateVector( vB, *pViewModel, vB );

				SetQuadRotation( i, vA, vB );
				iNumRotations = s_iNumRotations;
			}
		}
		else
		{
			for ( TINT i = 0; i < iNumRotations; i++ )
			{
				TASSERT( i < 16 );

				TFLOAT fAngle      = pQuad->m_afRotations[ i ] * a_fDeltaTime + m_afRotations[ i ];
				m_afRotations[ i ] = fAngle;

				TFLOAT fSin, fCos;
				TMath::SinCos( fAngle, fSin, fCos );
				fSin = fSin * 0.5f;
				fCos = fCos * 0.5f;

				// Rotated in the XZ plane
				TFLOAT    fNegSin  = -fSin;
				TVector4* pCorners = &s_avQuadRotations[ i * 4 ];
				pCorners[ 0 ].y    = 0.0f;
				pCorners[ 0 ].w    = 1.0f;
				pCorners[ 0 ].x    = fSin - fCos;
				pCorners[ 0 ].z    = fCos - fNegSin;
				pCorners[ 1 ].y    = 0.0f;
				pCorners[ 1 ].w    = 1.0f;
				pCorners[ 1 ].x    = fSin + fCos;
				pCorners[ 1 ].z    = fNegSin + fCos;
				pCorners[ 3 ].y    = 0.0f;
				pCorners[ 3 ].w    = 1.0f;
				pCorners[ 3 ].x    = fCos - fSin;
				pCorners[ 3 ].z    = fNegSin - fCos;
				pCorners[ 2 ].y    = -0.0f;
				pCorners[ 2 ].z    = -fCos - fNegSin;
				pCorners[ 2 ].w    = -1.0f;
				pCorners[ 2 ].x    = fNegSin - fCos;
				iNumRotations      = s_iNumRotations;
			}
		}
	}

	s_iNumRotations = NextPowerOf2( iNumRotations ); // inlined
}

// $deBlob: FUNCTION 00714b20
void TGenericParticles::UpdateLighting()
{
	TRender*        pRender  = TRender::GetSingleton();
	TRenderContext* pContext = pRender->GetCurrentContext();

	TVector4 vLight;
	vLight.Multiply( pRender->GetLightColour().AsBasisVector4( BASISVECTOR_RIGHT ), m_pSettings->m_fLightScale );

	const TVector4& rAmbient = pContext->GetAmbientColour();
	vLight.x                 = rAmbient.x + vLight.x;
	vLight.y                 = rAmbient.y + vLight.y;
	vLight.z                 = rAmbient.z + vLight.z;
	vLight.Multiply( vLight, 255.0f );

	TMath::Clip( vLight.x, 0.0f, 255.0f );
	TMath::Clip( vLight.y, 0.0f, 255.0f );
	TMath::Clip( vLight.z, 0.0f, 255.0f );

	s_iLightR = TINT( lrintf( vLight.x ) );
	s_iLightG = TINT( lrintf( vLight.y ) );
	s_iLightB = TINT( lrintf( vLight.z ) );
}

// $deBlob: FUNCTION 00714c30
void TGenericParticles::InitParticles( TINT a_iStart, TINT a_iEnd, TFLOAT a_fTimeOffset, TBOOL a_bInitLife, TFLOAT a_fBlend )
{
	if ( a_iStart >= a_iEnd )
		return;

	m_pSettings->InitParticles( this, a_iStart, a_iEnd, a_bInitLife );

	TUINT  uiFlags = m_uiFlags;
	TFLOAT fStep   = 0.0f;

	if ( ( uiFlags & ( FLAGS_TRANSWITHXFORM | FLAGS_TRANSROTWITHXFORM ) ) == 0 )
	{
		// The particles are emitted in world space: transform them
		fStep = 1.0f / TFLOAT( a_iEnd - a_iStart );

		if ( 1 < a_iEnd - a_iStart || 0.0f <= a_fBlend )
		{
			// Spread between the previous and the current transform
			TQuaternion qRotation;
			TMatrix44::MatToQuat( qRotation, m_oTransform );

			TFLOAT fBlend = a_fBlend;
			TINT   iIndex = 0;

			for ( TINT i = a_iStart; i < a_iEnd; i++, iIndex++ )
			{
				if ( a_fBlend < 0.0f )
				{
					fBlend = TFLOAT( iIndex ) * fStep;
					TASSERT( fBlend >= 0.0f && fBlend <= 1.0f );
				}

				TQuaternion qBlend;
				TQuaternion::Nlerp( qBlend, m_qPrevRotation, qRotation, fBlend );

				TMatrix44 oMatrix;
				oMatrix.SetFromQuaternion( qBlend );
				oMatrix.GetTranslation().Lerp4( m_vPrevTranslation, m_oTransform.GetTranslation(), fBlend );
				oMatrix.m_f44 = 1.0f;

				TVector4& rPosition = m_avPosition[ i ];
				TMatrix44::TransformVector( rPosition, oMatrix, rPosition );

				TVector4& rVelocity = m_avVelocity[ i ];
				TMatrix44::RotateVector( rVelocity, oMatrix, rVelocity );
			}
		}
		else
		{
			// One particle: with the current transform, a block of 32 at a time
			TINT i = a_iStart;

			do
			{
				TINT iBlockEnd = ( i & ~31 ) + 32;

				if ( a_iEnd < iBlockEnd )
					iBlockEnd = a_iEnd;

				TINT iCount = iBlockEnd - i;
				TASSERT( ( i / 32 ) == ( ( iBlockEnd - 1 ) / 32 ) && "(particleStart/32) == ((particleEnd-1)/32)" );

				TVector4* pPositions  = &m_avPosition[ i ];
				TVector4* pVelocities = &m_avVelocity[ i ];
				TMatrix44::TransformVectorBatched( pPositions, m_oTransform, pPositions, iCount );
				TMatrix44::RotateVectorBatched( pVelocities, m_oTransform, pVelocities, iCount );

				i = iBlockEnd;
			} while ( i < a_iEnd );
		}

		if ( 0.0f < a_fTimeOffset || !a_bInitLife )
		{
			for ( TINT i = a_iStart; i < a_iEnd; i++ )
			{
				TFLOAT fTime;

				if ( !a_bInitLife )
				{
					// The time the particle lived
					fTime = m_afLife[ i ] / m_afLifeLostPerSec[ i ];
				}
				else
				{
					fTime = a_fBlend;

					if ( a_fBlend < 0.0f )
					{
						fTime = TFLOAT( i - a_iStart ) * fStep;
						TASSERT( fTime >= 0.0f && fTime <= 1.0f && "fBlend >= 0.0f && fBlend <= 1.0f" );
					}

					fTime         = fTime * a_fTimeOffset;
					m_afLife[ i ] = m_afLifeLostPerSec[ i ] * fTime + m_afLife[ i ];
				}

				if ( m_pSettings->m_pColourKeys ) m_pSettings->m_pColourKeys->InitParticles( this, i, i + 1 );
				if ( m_pSettings->m_pUVKeys ) m_pSettings->m_pUVKeys->InitParticles( this, i, i + 1 );
				if ( m_pSettings->m_pMassKeys ) m_pSettings->m_pMassKeys->InitParticles( this, i, i + 1 );
				if ( m_pSettings->m_pSizeKeys ) m_pSettings->m_pSizeKeys->InitParticles( this, i, i + 1 );
				if ( m_pSettings->m_pEventKeys ) m_pSettings->m_pEventKeys->InitParticles( this, i, i + 1 );

				// The particle moves for the time it already lived
				for ( TParticleAffector* pAffector = m_pSettings->m_pAffectors; pAffector != TNULL; pAffector = pAffector->m_pNext )
					pAffector->m_fnUpdate( this, i, i + 1, fTime, pAffector->m_pData );
			}
		}
	}
	else if ( uiFlags & FLAGS_TRANSWITHXFORM )
	{
		// The particles are in the emitter's space, without its rotation: rotate them
		TASSERT( TFALSE == HasFlag( FLAGS_TRANSROTWITHXFORM ) );

		TINT i = a_iStart;

		do
		{
			TINT iBlockEnd = ( i & ~31 ) + 32;

			if ( a_iEnd < iBlockEnd )
				iBlockEnd = a_iEnd;

			TINT iCount = iBlockEnd - i;
			TASSERT( ( i / 32 ) == ( ( iBlockEnd - 1 ) / 32 ) && "(particleStart/32) == ((particleEnd-1)/32)" );

			TVector4* pPositions  = &m_avPosition[ i ];
			TVector4* pVelocities = &m_avVelocity[ i ];
			TMatrix44::RotateVectorBatched( pPositions, m_oTransform, pPositions, iCount );
			TMatrix44::RotateVectorBatched( pVelocities, m_oTransform, pVelocities, iCount );

			i = iBlockEnd;
		} while ( i < a_iEnd );
	}

	TRibbonRenderSettings* pRibbonSettings = m_pSettings->m_pRibbonSettings;

	if ( pRibbonSettings != TNULL && pRibbonSettings->m_uiForwardDir != 0 )
	{
		TASSERT( pRibbonSettings->m_uiForwardDir - TRibbonRenderSettings::RIBBONFORWARD_EMITTERX >= 0 );
		TASSERT( pRibbonSettings->m_uiForwardDir - TRibbonRenderSettings::RIBBONFORWARD_EMITTERX <= 2 );

		const TVector4& rForward = m_oTransform.AsBasisVector4( pRibbonSettings->m_uiForwardDir - 1 );

		for ( TINT i = a_iStart; i < a_iEnd; i++ )
		{
			TVector4 vForward  = rForward;
			m_avForward[ i ]   = vForward;
		}
	}
}

//-----------------------------------------------------------------------------
// The vertex writers
//-----------------------------------------------------------------------------

static void SetQuadPositions( TParticleMesh* a_pMesh, TINT a_iVertIndex, const TVector4& a_rPosition, const TVector4& a_rV0, const TVector4& a_rV1, const TVector4& a_rV2, const TVector4& a_rV3 )
{
	a_pMesh->SetPosition( a_iVertIndex, a_rV0, a_rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 1, a_rV1, a_rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 2, a_rV2, a_rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 3, a_rV3, a_rPosition );
}

// $deBlob: FUNCTION 00715240
void __stdcall TGenericParticles::SetPosition( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );

	TVector4 v0, v1, v2, v3;
	v0.Multiply( s_vBLOffset, 3.0f );
	v1.Multiply( s_vTLOffset, 3.0f );
	v2.Multiply( s_vBROffset, 3.0f );
	v3.Multiply( s_vTROffset, 3.0f );
	SetQuadPositions( a_pMesh, a_iVertIndex, pParticles->m_avPosition[ a_iParticle ], v0, v1, v2, v3 );
}

// $deBlob: FUNCTION 00715360
void __stdcall TGenericParticles::SetPositionSizeBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TFLOAT             fSize      = pParticles->m_fnGetSize( pParticles, a_iParticle );

	TVector4 v0, v1, v2, v3;
	v0.Multiply( s_vBLOffset, fSize );
	v1.Multiply( s_vTLOffset, fSize );
	v2.Multiply( s_vBROffset, fSize );
	v3.Multiply( s_vTROffset, fSize );
	SetQuadPositions( a_pMesh, a_iVertIndex, pParticles->m_avPosition[ a_iParticle ], v0, v1, v2, v3 );
}

// $deBlob: FUNCTION 007154a0
void __stdcall TGenericParticles::SetPositionSizeBlendRotate( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles    = TSTATICCAST( TGenericParticles, a_pParticles );
	TINT               iNumRotations = s_iNumRotations;
	TASSERT( ( 1 << FloatIntLog2( iNumRotations ) ) == iNumRotations && "(1<<TMath::IntLog2(iNumRotations)) == iNumRotations" );

	TINT   iRotation = a_iParticle & ( iNumRotations - 1 );
	TFLOAT fSize     = pParticles->m_fnGetSize( pParticles, a_iParticle );

	TVector4 v0, v1, v2, v3;
	v0.Multiply( s_avQuadRotations[ iRotation * 4 + 2 ], fSize );
	v1.Multiply( s_avQuadRotations[ iRotation * 4 + 0 ], fSize );
	v2.Multiply( s_avQuadRotations[ iRotation * 4 + 3 ], fSize );
	v3.Multiply( s_avQuadRotations[ iRotation * 4 + 1 ], fSize );
	SetQuadPositions( a_pMesh, a_iVertIndex, pParticles->m_avPosition[ a_iParticle ], v0, v1, v2, v3 );
}

// $deBlob: FUNCTION 00715650
void __stdcall TGenericParticles::SetPositionConstantSizeBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TFLOAT             fSize      = TMath::LERP( s_fConstSize0, s_fConstSize1, pParticles->m_afLife[ a_iParticle ] );

	TVector4 v0, v1, v2, v3;
	v0.Multiply( s_vBLOffset, fSize );
	v1.Multiply( s_vTLOffset, fSize );
	v2.Multiply( s_vBROffset, fSize );
	v3.Multiply( s_vTROffset, fSize );
	SetQuadPositions( a_pMesh, a_iVertIndex, pParticles->m_avPosition[ a_iParticle ], v0, v1, v2, v3 );
}

// $deBlob: FUNCTION 007157c0
void __stdcall TGenericParticles::SetPositionSpark( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles*    pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TFLOAT                fSize      = pParticles->m_fnGetSize( pParticles, a_iParticle );
	const TVector4&       rPosition  = pParticles->m_avPosition[ a_iParticle ];
	const TVector4&       rVelocity  = pParticles->m_avVelocity[ a_iParticle ];
	TSparkRenderSettings* pSpark     = pParticles->m_pSettings->m_pSparkSettings;
	TFLOAT                fWidth     = pSpark->m_fWidth;
	TFLOAT                fLength    = pSpark->m_fLength;

	TVector4 vDir  = rVelocity;
	TFLOAT   fTail = -( pSpark->m_fTailLength * fSize );
	vDir.Normalise();

	TVector4 vSide;
	vSide.Multiply( vDir, fWidth * fSize );
	vSide.CrossProduct( vSide, TVector4::VEC_NEGY );

	TVector4 v0( vSide.x, vSide.y, vSide.z, vSide.w );
	TVector4 v3( -vSide.x, -vSide.y, -vSide.z, vSide.w );

	TVector4 v1, v2;
	v1.Multiply( vDir, fLength * fSize );
	v2.Multiply( vDir, fTail );

	a_pMesh->SetPosition( a_iVertIndex, v0, rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 1, v1, rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 2, v2, rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 3, v3, rPosition );
}

// $deBlob: FUNCTION 007159a0
void __stdcall TGenericParticles::SetPositionSparkQuad( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles*        pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TFLOAT                    fSize      = pParticles->m_fnGetSize( pParticles, a_iParticle );
	const TVector4&           rPosition  = pParticles->m_avPosition[ a_iParticle ];
	const TVector4&           rVelocity  = pParticles->m_avVelocity[ a_iParticle ];
	TSparkQuadRenderSettings* pSpark     = pParticles->m_pSettings->m_pSparkQuadSettings;

	TVector4 vDir    = rVelocity;
	TFLOAT   fWidth  = pSpark->m_fWidth;
	TFLOAT   fLength = pSpark->m_fLength * fSize;
	vDir.Normalise();

	TVector4 vSide;
	vSide.Multiply( vDir, fWidth * fSize );
	vSide.CrossProduct( vSide, TVector4::VEC_NEGY );

	TFLOAT fNegSideX = -vSide.x;
	TFLOAT fNegSideY = -vSide.y;
	TFLOAT fNegSideZ = -vSide.z;

	TVector4 vForward;
	vForward.Multiply( vDir, fLength );

	TVector4 v0( vForward.x - vSide.x, vForward.y - vSide.y, vForward.z - vSide.z, 0.0f );
	TVector4 v2( vForward.x - fNegSideX, vForward.y - fNegSideY, vForward.z - fNegSideZ, 0.0f );
	TVector4 v1( -vForward.x - vSide.x, -vForward.y - vSide.y, -vForward.z - vSide.z, 0.0f );
	TVector4 v3( -vForward.x - fNegSideX, -vForward.y - fNegSideY, -vForward.z - fNegSideZ, 0.0f );

	a_pMesh->SetPosition( a_iVertIndex, v0, rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 1, v1, rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 2, v2, rPosition );
	a_pMesh->SetPosition( a_iVertIndex + 3, v3, rPosition );
}

// $deBlob: FUNCTION 00715c00
void __stdcall TGenericParticles::SetPositionRibbon( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles, const TVector4* a_pUp )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TFLOAT             fSize      = pParticles->m_afSize[ a_iParticle ];
	const TVector4&    rPosition  = pParticles->m_avPosition[ a_iParticle ];

	TVector4 vOffset;
	vOffset.Multiply( *a_pUp, fSize );
	a_pMesh->SetPosition( a_iVertIndex, vOffset, rPosition );

	vOffset.x = -vOffset.x;
	vOffset.y = -vOffset.y;
	vOffset.z = -vOffset.z;
	a_pMesh->SetPosition( a_iVertIndex + 1, vOffset, rPosition );
}

// $deBlob: FUNCTION 00715d10
void __stdcall TGenericParticles::SetConstantUV( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	a_pMesh->SetUV( a_iVertIndex, &pParticles->m_afConstantUVs[ 0 ] );
	a_pMesh->SetUV( a_iVertIndex + 1, &pParticles->m_afConstantUVs[ 2 ] );
	a_pMesh->SetUV( a_iVertIndex + 2, &pParticles->m_afConstantUVs[ 4 ] );
	a_pMesh->SetUV( a_iVertIndex + 3, &pParticles->m_afConstantUVs[ 6 ] );
}

// $deBlob: FUNCTION 00715de0
void __stdcall TGenericParticles::SetConstantUVRibbon( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TFLOAT             fLife      = pParticles->m_afLife[ a_iParticle ];

	if ( 1.0f < fLife )
		fLife = 1.0f;

	TFLOAT fV = ( fLife - s_fYoungestLife ) * s_fOldestLife;
	a_pMesh->SetUV( a_iVertIndex, 0.0f, fV );
	a_pMesh->SetUV( a_iVertIndex + 1, 1.0f, fV );
}

// $deBlob: FUNCTION 00715ed0
void __stdcall TGenericParticles::SetUVFromUVIndexRotate( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles*         pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TGenericParticlesSettings* pSettings  = pParticles->m_pSettings;
	TParticleMaterial*         pMaterial  = pParticles->m_pMaterial;
	const TVector4*            pUVRect    = pMaterial->GetUVRect( pParticles->m_aiUVIndex[ a_iParticle ] );

	TFLOAT fAngle   = pSettings->m_pQuadSettings->m_fUVRotation * 0.017453292f;
	TFLOAT fHalfU   = ( pUVRect->z - pUVRect->x ) * 0.5f;
	TFLOAT fHalfV   = ( pUVRect->w - pUVRect->y ) * 0.5f;
	TFLOAT fU1      = pUVRect->x - fHalfU;
	TFLOAT fU2      = pUVRect->z - fHalfU;
	TFLOAT fV1      = pUVRect->y - fHalfV;
	TFLOAT fV2      = pUVRect->w - fHalfV;
	TFLOAT fCos     = TFLOAT( cos( TDOUBLE( fAngle ) ) );
	TFLOAT fSin     = TFLOAT( sin( TDOUBLE( fAngle ) ) );

	a_pMesh->SetUV( a_iVertIndex, ( fCos * fU1 - fSin * fV1 ) + fHalfU, fCos * fV1 + fSin * fU1 + fHalfV );
	a_pMesh->SetUV( a_iVertIndex + 1, ( fCos * fU1 - fSin * fV2 ) + fHalfU, fCos * fV2 + fSin * fU1 + fHalfV );
	a_pMesh->SetUV( a_iVertIndex + 2, ( fCos * fU2 - fSin * fV1 ) + fHalfU, fSin * fU2 + fCos * fV1 + fHalfV );
	a_pMesh->SetUV( a_iVertIndex + 3, ( fCos * fU2 - fSin * fV2 ) + fHalfU, fCos * fV2 + fSin * fU2 + fHalfV );
}

// $deBlob: FUNCTION 00716190
void __stdcall TGenericParticles::SetUVFromUVIndex( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	const TVector4*    pUVRect    = pParticles->m_pMaterial->GetUVRect( pParticles->m_aiUVIndex[ a_iParticle ] );

	a_pMesh->SetUV( a_iVertIndex, pUVRect->x, pUVRect->y );
	a_pMesh->SetUV( a_iVertIndex + 1, pUVRect->x, pUVRect->w );
	a_pMesh->SetUV( a_iVertIndex + 2, pUVRect->z, pUVRect->y );
	a_pMesh->SetUV( a_iVertIndex + 3, pUVRect->z, pUVRect->w );
}

// $deBlob: FUNCTION 007162f0
void __stdcall TGenericParticles::SetUVFromUVIndexRibbon( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	const TVector4*    pUVRect    = pParticles->m_pMaterial->GetUVRect( pParticles->m_aiUVIndex[ a_iParticle ] );

	a_pMesh->SetUV( a_iVertIndex, pUVRect->x, pUVRect->w );
	a_pMesh->SetUV( a_iVertIndex + 1, pUVRect->z, pUVRect->w );
}

// $deBlob: FUNCTION 00716410
void __stdcall TGenericParticles::SetUVFromUVIndexSparks( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	const TVector4*    pUVRect    = pParticles->m_pMaterial->GetUVRect( pParticles->m_aiUVIndex[ a_iParticle ] );

	// NOTE: retail adds the start to the middle (( u1 + u2 ) * 0.5 + u1), so the middle UVs are past the rect
	TFLOAT fU1 = pUVRect->x;
	TFLOAT fV  = ( pUVRect->y + pUVRect->w ) * 0.5f + pUVRect->y;
	TFLOAT fU  = ( pUVRect->z + fU1 ) * 0.5f + fU1;

	a_pMesh->SetUV( a_iVertIndex, fU1, fV );
	a_pMesh->SetUV( a_iVertIndex + 1, fU, pUVRect->y );
	a_pMesh->SetUV( a_iVertIndex + 2, fU, pUVRect->w );
	a_pMesh->SetUV( a_iVertIndex + 3, pUVRect->z, fV );
}

// $deBlob: FUNCTION 007165a0
void __stdcall TGenericParticles::SetConstantColour( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TUINT32 uiColour = s_uiConstCol0;
	a_pMesh->SetColour( a_iVertIndex, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 1, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 2, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 3, uiColour );
}

// Lerps each channel of a_uiFrom to a_uiTo by a_iT (0-255)
static TUINT32 BlendColour( TUINT32 a_uiFrom, TUINT32 a_uiTo, TINT a_iT )
{
	TUINT uiB0 = a_uiFrom & 0xFF;
	TUINT uiG0 = ( a_uiFrom >> 8 ) & 0xFF;
	TUINT uiR0 = ( a_uiFrom >> 16 ) & 0xFF;
	TUINT uiA0 = a_uiFrom >> 24;

	return ( ( TINT( ( ( a_uiTo >> 8 ) & 0xFF ) - uiG0 ) * a_iT >> 8 ) + uiG0 ) * 0x100 |
	    ( ( TINT( ( ( a_uiTo >> 16 ) & 0xFF ) - uiR0 ) * a_iT >> 8 ) + uiR0 ) * 0x10000 |
	    ( ( TINT( ( a_uiTo >> 24 ) - uiA0 ) * a_iT >> 8 ) + uiA0 ) * 0x1000000 |
	    ( ( TINT( ( a_uiTo & 0xFF ) - uiB0 ) * a_iT >> 8 ) + uiB0 );
}

// The colour of a particle with the render context's lighting (the alpha isn't lit)
static TUINT32 LightColour( TUINT32 a_uiColour )
{
	TINT iR = ( ( a_uiColour >> 16 ) & 0xFF ) * TGenericParticles::s_iLightR;
	TINT iG = ( ( a_uiColour >> 8 ) & 0xFF ) * TGenericParticles::s_iLightG;
	TINT iB = ( a_uiColour & 0xFF ) * TGenericParticles::s_iLightB;

	return ( ( ( iR >> 8 ) & 0xFFU ) << 8 | ( ( iG >> 8 ) & 0xFFU ) ) << 8 | ( ( iB >> 8 ) & 0xFFU ) | ( a_uiColour & 0xFF000000 );
}

// The keyframed colour of a particle (the buffer's if it has no key yet)
static TUINT32 GetKeyframeColour( TGenericParticles* a_pParticles, TINT a_iParticle )
{
	TParticleKeyframe* pKey = TREINTERPRETCAST( TParticleKeyframe*, a_pParticles->m_apCurColourKey[ a_iParticle ] );

	if ( pKey == TNULL )
		return a_pParticles->m_auiColour[ a_iParticle ];

	if ( pKey->GetType() == TParticleKeyframe::KEYFRAMETYPE_ALPHA )
		return pKey->GetAlpha( a_pParticles, a_iParticle );

	return pKey->GetColour( a_pParticles, a_iParticle );
}

// $deBlob: FUNCTION 00716940
void __stdcall TGenericParticles::SetColourFromBuffer( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TUINT32            uiColour   = pParticles->m_auiColour[ a_iParticle ];

	for ( TUINT i = 0; i < TUINT( pParticles->GetVertsPerNumParticles( 1 ) ); i++ )
		a_pMesh->SetColour( i + a_iVertIndex, uiColour );
}

// $deBlob: FUNCTION 00716a00
void __stdcall TGenericParticles::SetColourFromKeyframe( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TUINT32            uiColour   = GetKeyframeColour( pParticles, a_iParticle );

	for ( TUINT i = 0; i < TUINT( pParticles->GetVertsPerNumParticles( 1 ) ); i++ )
		a_pMesh->SetColour( a_iVertIndex + i, uiColour );
}

// $deBlob: FUNCTION 00716af0
void __stdcall TGenericParticles::SetColourFromBufferWithLighting( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TUINT32            uiColour   = LightColour( pParticles->m_auiColour[ a_iParticle ] );

	for ( TUINT i = 0; i < TUINT( pParticles->GetVertsPerNumParticles( 1 ) ); i++ )
		a_pMesh->SetColour( a_iVertIndex + i, uiColour );
}

// $deBlob: FUNCTION 00716c10
void __stdcall TGenericParticles::SetColourFromKeyframeWithLighting( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TUINT32            uiColour   = LightColour( GetKeyframeColour( pParticles, a_iParticle ) );

	for ( TUINT i = 0; i < TUINT( pParticles->GetVertsPerNumParticles( 1 ) ); i++ )
		a_pMesh->SetColour( a_iVertIndex + i, uiColour );
}

// $deBlob: FUNCTION 00716650
void __stdcall TGenericParticles::SetConstantColourBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TINT               iT         = TINT( lrintf( pParticles->m_afLife[ a_iParticle ] * 255.0f ) );
	TUINT32            uiColour   = BlendColour( s_uiConstCol0, s_uiConstCol1, iT );

	a_pMesh->SetColour( a_iVertIndex, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 1, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 2, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 3, uiColour );
}

// $deBlob: FUNCTION 007167c0
void __stdcall TGenericParticles::SetColourBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles )
{
	TASSERT( a_iVertIndex >= 0 );
	TASSERT( a_iParticle >= 0 );

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, a_pParticles );
	TUINT32            uiFrom     = pParticles->m_auiColour[ a_iParticle ];
	TINT               iT         = TINT( lrintf( pParticles->m_afLife[ a_iParticle ] * 255.0f ) );
	TUINT32            uiColour   = BlendColour( uiFrom, s_uiConstCol1, iT );

	a_pMesh->SetColour( a_iVertIndex, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 1, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 2, uiColour );
	a_pMesh->SetColour( a_iVertIndex + 3, uiColour );
}

// $deBlob: FUNCTION 00716d50
TFLOAT __stdcall TGenericParticles::GetSizeFixed( TGenericParticles* a_pParticles, TINT a_iParticle )
{
	return a_pParticles->m_afSize[ a_iParticle ];
}

// $deBlob: FUNCTION 00716d70
TFLOAT __stdcall TGenericParticles::GetSizeKeyframe( TGenericParticles* a_pParticles, TINT a_iParticle )
{
	TParticleKeyframe* pKey = TREINTERPRETCAST( TParticleKeyframe*, a_pParticles->m_apCurSizeKey[ a_iParticle ] );

	if ( pKey == TNULL )
		return a_pParticles->m_afSize[ a_iParticle ];

	return pKey->GetSize( a_pParticles, a_iParticle );
}

// $deBlob: FUNCTION 00716db0
void TGenericParticles::RenderVertsRibbon( t_SetVertsFn a_fnSetPosition, t_SetVertsFn a_fnSetUV, t_SetVertsFn a_fnSetColour, t_SetIndicesFn a_fnSetIndices, TParticles* a_pParticles )
{
	// NOTE: a_fnSetPosition is unused, the ribbons' positions are always written by SetPositionRibbon
	if ( m_iNumRenderingParticles <= 1 )
		return;

	TINT                   iNumVertsLeft   = m_iNumRenderingParticles * 4;
	TBOOL                  bDone           = TFALSE;
	TINT                   iParticle       = 0;
	TRibbonRenderSettings* pRibbonSettings = m_pSettings->m_pRibbonSettings;

	if ( pRibbonSettings->m_uiUpDir == 0 )
	{
		s_vRibbonUp.x = pRibbonSettings->m_vUp.x;
		s_vRibbonUp.y = pRibbonSettings->m_vUp.y;
		s_vRibbonUp.z = pRibbonSettings->m_vUp.z;
	}
	else
	{
		TASSERT( pRibbonSettings->m_uiUpDir - TRibbonRenderSettings::RIBBONUP_EMITTERX >= 0 );
		TASSERT( pRibbonSettings->m_uiUpDir - TRibbonRenderSettings::RIBBONUP_EMITTERX <= 2 );
		s_vRibbonUp = m_oTransform.AsBasisVector4( BASISVECTOR( pRibbonSettings->m_uiUpDir - 1 ) );
	}

	// The particles sorted by their life (youngest first)
	TUINT8 aiBuf[ 201 ];
	TASSERT( m_iNumRenderingParticles + 1 <= sizeof( aiBuf ) );

	for ( TINT i = 0; i < m_iNumRenderingParticles; i++ )
		aiBuf[ i ] = TUINT8( i );

	for ( TINT i = 0; i < m_iNumRenderingParticles - 1; i++ )
	{
		TFLOAT fMinLife = m_afLife[ aiBuf[ i ] ];

		for ( TINT j = i + 1; j < m_iNumRenderingParticles; j++ )
		{
			TFLOAT fLife = m_afLife[ aiBuf[ j ] ];

			if ( fLife < fMinLife )
			{
				TUINT8 uiTemp = aiBuf[ i ];
				aiBuf[ i ]    = aiBuf[ j ];
				aiBuf[ j ]    = uiTemp;
				fMinLife      = fLife;
			}
		}
	}

	s_fYoungestLife                     = m_afLife[ aiBuf[ 0 ] ];
	s_fRibbonOldestLife                 = m_afLife[ aiBuf[ m_iNumRenderingParticles - 1 ] ];
	aiBuf[ m_iNumRenderingParticles ] = aiBuf[ m_iNumRenderingParticles - 1 ];

	TFLOAT fOldest = s_fRibbonOldestLife;
	if ( 1.0f < fOldest )
		fOldest = 1.0f;

	s_fOldestLife = 1.0f / ( fOldest - s_fYoungestLife );

	TVector4 vUp;

	if ( pRibbonSettings->m_bAttachToEmitter )
	{
		// The youngest particle sticks to the emitter
		vUp                          = m_oTransform.GetTranslation();
		m_avPosition[ aiBuf[ 0 ] ] = vUp;

		if ( HasFlag( FLAGS_REEMIT ) )
			m_afLife[ aiBuf[ 0 ] ] = 0.0f;
	}

	while ( TTRUE )
	{
		TParticleMesh* pMesh = TParticleSystem::GetSingleton()->GetMesh( iNumVertsLeft, GetVertsPerNumParticles( 1 ), m_pMaterial );

		if ( pMesh == TNULL )
			break;

		pMesh->SetMeshFlags( 0x80, TFALSE );
		pMesh->SetMeshFlags( 0x100, TTRUE );

		TINT iNumParticles = pMesh->GetMaxVertices() / GetVertsPerNumParticles( 1 );

		if ( iNumParticles == 0 )
			break;

		if ( m_iNumRenderingParticles <= iNumParticles + iParticle )
		{
			bDone         = TTRUE;
			iNumParticles = m_iNumRenderingParticles - iParticle;
		}

		iNumVertsLeft -= GetVertsPerNumParticles( iNumParticles );

		for ( TINT i = 0; i < iNumParticles; i++ )
		{
			TINT   iSorted = iParticle + i;
			TUINT8 uiParticle = aiBuf[ iSorted ];

			if ( pRibbonSettings->m_uiForwardDir == 0 )
			{
				if ( iSorted + 1 != m_iNumRenderingParticles )
				{
					const TVector4& rPos  = m_avPosition[ uiParticle ];
					const TVector4& rNext = m_avPosition[ aiBuf[ iSorted + 1 ] ];

					TVector4 vDir;
					vDir.x = rNext.x - rPos.x;
					vDir.y = rNext.y - rPos.y;
					vDir.z = rNext.z - rPos.z;
					vDir.w = rNext.w;
					vDir.Normalise();
					vUp.CrossProduct( vDir, s_vRibbonUp );
				}
			}
			else
			{
				vUp = m_avForward[ uiParticle ];
			}

			TINT iVertIndex = i * 2;
			a_fnSetColour( pMesh, iVertIndex, uiParticle, a_pParticles );
			a_fnSetUV( pMesh, iVertIndex, uiParticle, a_pParticles );
			SetPositionRibbon( pMesh, iVertIndex, uiParticle, a_pParticles, &vUp );
		}

		iParticle += iNumParticles;

		TASSERT( GetVertsPerNumParticles( iNumParticles ) == (TUINT32)( iNumParticles << 1 ) );
		TINT iNumVerts   = GetVertsPerNumParticles( iNumParticles );
		TINT iNumIndices = a_fnSetIndices( pMesh, iNumParticles, a_pParticles );
		pMesh->Unlock( iNumVerts, iNumIndices );

		if ( bDone )
			break;

		// The next mesh starts with the last particle of this one
		iParticle--;
	}
}

// $deBlob: FUNCTION 00717410
TINT __stdcall TGenericParticles::RenderRibbonIndices( TParticleMesh* a_pMesh, TINT a_iNumParticles, TParticles* a_pParticles )
{
	TASSERT( a_iNumParticles > 0 );

	TUINT uiVert  = 0;
	TUINT uiIndex = 1;
	a_pMesh->SetIndex( 0, 0 );

	for ( TINT i = 0; i < a_iNumParticles; i++ )
	{
		a_pMesh->SetIndex( uiIndex, uiVert );
		a_pMesh->SetIndex( uiIndex + 1, uiVert + 1 );
		uiIndex += 2;
		uiVert += 2;
	}

	a_pMesh->SetIndex( uiIndex, uiVert - 1 );
	return ( uiIndex + 1 ) & 0xFFFF;
}

// $deBlob: FUNCTION 00717560
void TGenericParticles::RenderVertsMesh()
{
	if ( m_iNumRenderingParticles < 1 )
		return;

	T2ModelPtr pModel( m_pSettings->m_pModel );
	TASSERT( pModel.IsValid() && "Attempting to render undefined particle model" );
	TASSERT( TINT( pModel->GetResourcePtr() ) != T2ResourcePtr::IDINVALID && "Attempting to render broken particle model" );

	TRender*        pRender  = TRender::GetSingleton();
	TRenderContext* pContext = pRender->GetCurrentContext();
	TASSERT( pModel->GetLODCount() >= 1 );

	TModelLOD& rLOD = pModel->GetLOD( 0 );

	TMatrix44 oModelView    = pContext->GetModelViewMatrix();
	TVector4  vAmbient      = pContext->GetAmbientColour();
	TFLOAT    fAlphaBlend   = pContext->GetAlphaBlend();
	TMatrix44 oLightColour  = pRender->GetLightColour();
	TINT      iNumMeshes    = rLOD.GetMeshCount();
	TBOOL     bLighting     = ( m_pSettings->m_uiFlags & TGenericParticlesSettings::FLAGS_LIGHTING ) != 0;
	TBOOL     bColourKeys   = m_pSettings->m_pColourKeys != TNULL;
	TMatrix44 oParticleLight = oLightColour;

	if ( !bLighting )
	{
		TVector4 vNoLight( 0.0f, 0.0f, 0.0f, 1.0f );
		oParticleLight.AsBasisVector4( BASISVECTOR( 0 ) ) = vNoLight;
		oParticleLight.AsBasisVector4( BASISVECTOR( 1 ) ) = vNoLight;
		oParticleLight.AsBasisVector4( BASISVECTOR( 2 ) ) = vNoLight;
		oParticleLight.AsBasisVector4( BASISVECTOR( 3 ) ) = vNoLight;
		pRender->SetLightColourMatrix( oParticleLight );
	}

	for ( TINT i = 0; i < m_iNumRenderingParticles; i++ )
	{
		// The particle's matrix: forward against the velocity, scaled by the size
		TMatrix44 oParticle;
		TVector4& rRight   = oParticle.AsBasisVector4( BASISVECTOR( 0 ) );
		TVector4& rUp      = oParticle.AsBasisVector4( BASISVECTOR( 1 ) );
		TVector4& rForward = oParticle.AsBasisVector4( BASISVECTOR( 2 ) );
		TVector4& rPos     = oParticle.AsBasisVector4( BASISVECTOR( 3 ) );

		rPos = m_avPosition[ i ];

		const TVector4& rVelocity = m_avVelocity[ i ];
		rForward.x                = -rVelocity.x;
		rForward.y                = -rVelocity.y;
		rForward.z                = -rVelocity.z;
		rForward.w                = rVelocity.w;
		rForward.Normalise();

		rRight.CrossProduct( ( TMath::Abs( rForward.z ) < 0.5f ) ? TVector4::VEC_POSZ : TVector4::VEC_POSY, rForward );
		rUp.CrossProduct( rRight, rForward );
		rRight.w   = 0.0f;
		rUp.w      = 0.0f;
		rForward.w = 0.0f;
		rPos.w     = 1.0f;

		TFLOAT fSize = m_fnGetSize( this, i );
		oParticle.Scale( fSize, fSize, fSize );

		TMatrix44 oMatrix;
		oMatrix.Multiply( oModelView, oParticle );
		pContext->SetModelViewMatrix( oMatrix );

		// The colour
		TParticleKeyframe* pKey = TREINTERPRETCAST( TParticleKeyframe*, m_apCurColourKey[ i ] );
		TUINT32            uiColour;

		if ( pKey == TNULL || !bColourKeys )
			uiColour = m_auiColour[ i ];
		else if ( pKey->GetType() == TParticleKeyframe::KEYFRAMETYPE_ALPHA )
			uiColour = pKey->GetAlpha( this, i );
		else
			uiColour = pKey->GetColour( this, i );

		TVector4 vColour;
		vColour.z = ( TFLOAT( uiColour & 0xFF ) + 0.0f ) * ( 1.0f / 255.0f );
		vColour.y = ( TFLOAT( ( uiColour >> 8 ) & 0xFF ) + 0.0f ) * ( 1.0f / 255.0f );
		vColour.x = ( TFLOAT( ( uiColour >> 16 ) & 0xFF ) + 0.0f ) * ( 1.0f / 255.0f );
		vColour.w = ( TFLOAT( uiColour >> 24 ) + 0.0f ) * ( 1.0f / 255.0f );

		if ( !bLighting )
		{
			pContext->SetAmbientColour( vColour );
		}
		else
		{
			TVector4 vLitAmbient;
			vLitAmbient.Multiply( vAmbient, vColour );
			oParticleLight.AsBasisVector4( BASISVECTOR( 0 ) ).Multiply( oLightColour.AsBasisVector4( BASISVECTOR( 0 ) ), vColour );
			pContext->SetAmbientColour( vLitAmbient );
			pRender->SetLightColourMatrix( oParticleLight );
		}

		pContext->SetAlphaBlend( vColour.w );

		for ( TINT j = 0; j < iNumMeshes; j++ )
			rLOD.GetMeshes()[ j ]->Render();
	}

	pContext->SetSkeletonInstance( TNULL );
	pContext->SetAmbientColour( vAmbient );
	pContext->SetAlphaBlend( fAlphaBlend );
	pRender->SetLightColourMatrix( oLightColour );
	pContext->SetModelViewMatrix( oModelView );
}

TOSHI_NAMESPACE_END
