#pragma once
#include <Render/TShader.h>
#include <Render/TMesh.h>
#include <Render/TMaterial.h>
#include <Render/TOrderTable.h>
#include <Toshi/T2DList.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>

#include <d3d11.h>

class ACamera;

//-----------------------------------------------------------------------------
// Purpose: the paint splats of the paint bots and the ink (singleton s_Instance). Particles fly with a gravity
// (m_fGravity) in 50 slots, land on a 20x20 grid of paint cells (m_pPaintCells, paint alpha in the top byte) and the
// cells are drawn as lines from a dynamic vertex buffer. Created by ARenderer::Create, rendered from
// ARenderer::MainScene (AddRenderCall) through its order table.
//-----------------------------------------------------------------------------
class APaintParticleSystem
    : public Toshi::TShader
{
public:
	//-----------------------------------------------------------------------------
	// The material of the paint (0x54 bytes, a TMaterial with no texture): the paint is registered in the order table
	// with this material.
	//-----------------------------------------------------------------------------
	class DummyMaterial
	    : public Toshi::TMaterial
	{
	public:
		friend class APaintParticleSystem; // the owner shader and the registered material

		DummyMaterial();
		~DummyMaterial();

		// $deBlob: FUNCTION 005fbac0
		virtual void OnDestroy() OVERRIDE { Toshi::TMaterial::OnDestroy(); } // +0x0C
	};

	//-----------------------------------------------------------------------------
	// The mesh of the paint (0x10 bytes, a TMesh): the render packet of the paint refers to it.
	//-----------------------------------------------------------------------------
	class DummyMesh
	    : public Toshi::TMesh
	{
	public:
		friend class APaintParticleSystem; // the owner shader

		DummyMesh();
		~DummyMesh();

		virtual void OnDestroy() OVERRIDE; // +0x1C
	};

	// A particle (0x48 bytes): a node of one of the three lists of the system
	struct Particle
	    : public Toshi::T2GenericDList::Node
	{
		// $deBlob: FUNCTION 005fb758
		Particle()
		    : m_vPosition( 0.0f, 0.0f, 0.0f, 1.0f )
		    , m_vVelocity( 0.0f, 0.0f, 0.0f, 1.0f )
		    , m_uiColour( 0xFFFFFFFF )
		    , m_fRadius( 1.0f )
		    , m_fLifeTime( 0.0f )
		    , m_bDecal( 0 )
		    , m_iParam38( 0 )
		    , m_iLineIndex( -1 )
		    , m_iParam40( 0 )
		    , m_iParam44( 0 )
		{}

		Toshi::TVector4 m_vPosition;  // 0x08
		Toshi::TVector4 m_vVelocity;  // 0x18
		TUINT32         m_uiColour;   // 0x28
		TFLOAT          m_fRadius;    // 0x2C
		TFLOAT          m_fLifeTime;  // 0x30 (2.0f on creation)
		TINT            m_bDecal;     // 0x34 (non-zero: the decal list, zero: the used list)
		TINT            m_iParam38;   // 0x38
		TINT            m_iLineIndex; // 0x3C (-1: no line)
		TINT            m_iParam40;   // 0x40
		TINT            m_iParam44;   // 0x44
	};

	TSTATICASSERT( sizeof( Particle ) == 0x48 );

public:
	APaintParticleSystem( TFLOAT a_fGravity );
	~APaintParticleSystem();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void         OnDestroy() OVERRIDE;                                     // +0x0C
	virtual void         StartFlush() OVERRIDE;                                    // +0x14
	virtual void         EndFlush() OVERRIDE;                                      // +0x18
	virtual TBOOL        Create() OVERRIDE;                                        // +0x1C
	virtual const TCHAR* GetShaderPrefix() OVERRIDE;                               // +0x28
	virtual void         Render( Toshi::TRenderPacket* a_pRenderPacket ) OVERRIDE; // +0x34

	// Takes a particle from the free list (else the used list, else the decal list), fills it and links it into the
	// decal list (a_bDecal) or the used list. Returns its index in m_aParticles, -1 when there is none.
	TINT AddParticle( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rVelocity, TUINT32 a_uiColour, TFLOAT a_fRadius, TINT a_iDecal, TINT a_iParam38, TINT a_iParam40, TINT a_iParam44 );

	// Unlinks a particle from the used or decal list and puts it back into the free list
	void ReleaseParticle( Particle* a_pParticle );

	// Clears the lists and the paint, and returns every particle to the free list
	void Reset();

	// Adds the paint of the viewport of the camera to the order table (ARenderer::MainScene)
	void AddRenderCall( ACamera* a_pCamera );

	// Fades the paint by a_fDeltaTime and updates the particles
	void FadePaint( TFLOAT a_fDeltaTime );

	// $deBlob: FUNCTION 006445b0
	void SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }

	// Moves one particle, writes its trail into the paint buffers and, for a decal particle, lands it on the world
	void UpdateParticle( Particle* a_pParticle, TFLOAT a_fDeltaTime, TFLOAT a_fDeltaTime2 );

public:
	static APaintParticleSystem* s_Instance; // 0x9A1790

private:
	static const TINT MAX_PARTICLES = 50;
	static const TINT MAX_LINES     = 200;

	Particle                 m_aParticles[ MAX_PARTICLES ]; // 0x0C
	Toshi::T2DList<Particle> m_lUsedList;                   // 0xE1C (3612)
	Toshi::T2DList<Particle> m_lDecalList;                  // 0xE24 (3620)
	Toshi::T2DList<Particle> m_lFreeList;                   // 0xE2C (3628)
	TFLOAT                   m_fGravity;                    // 0xE34 (3636), 80.0f from ARenderer::Create
	TBOOL                    m_bEnabled;                    // 0xE38 (3640)
	DummyMaterial            m_DummyMaterial;               // 0xE3C (3644)
	DummyMesh                m_DummyMesh;                   // 0xE90 (3728)
	Toshi::TOrderTable       m_OrderTable;                  // 0xEA0 (3744)
	Toshi::TMatrix44         m_mMatrix;                     // 0xEC8 (3784), the shake matrix of the camera
	TBOOL                    m_bDirty;                      // 0xF08 (3848)
	TFLOAT                   m_fFadeTimer;                  // 0xF0C (3852)
	ID3D11Buffer*            m_pVertexBuffer;               // 0xF10 (3856), 6400 bytes, dynamic
	void*                    m_pPaintBufferA;               // 0xF14 (3860), 400 cells of 12 bytes
	void*                    m_pPaintBufferB;               // 0xF18 (3864), 400 cells of 12 bytes (swapped by FadePaint)
	TUINT32*                 m_pPaintCells;                 // 0xF1C (3868), 400 cells of 4 bytes (byte 3: paint alpha)
	TINT                     m_iLineCursor;                 // 0xF20 (3872), ring cursor of the line list
	TUINT16*                 m_pLineIndices;                // 0xF24 (3876), the cells with a line (MAX_LINES)
	TINT                     m_iLineCount;                  // 0xF28 (3880)
	ID3D11VertexShader*      m_pVertexShader;               // 0xF2C (3884)
	ID3D11PixelShader*       m_pPixelShader;                // 0xF30 (3888), CreatePixelShader in Create, PSSetShader in Render
	ID3D11InputLayout*       m_pInputLayout;                // 0xF34 (3892), CreateInputLayout in Create, IASetInputLayout in Render
};                                                          // 0xF38

TSTATICASSERT( sizeof( APaintParticleSystem ) == 0xF38 );
