#pragma once
#include "TParticles.h"
#include "TParticleKeyframe.h"
#include "Math/TMatrix44.h"
#include "Math/TQuaternion.h"


TOSHI_NAMESPACE_START

class TGenericParticlesSettings;
class TParticleEffectInstance;

// The particles of a TGenericParticlesSettings (0x230 bytes), pooled by TParticleSystem
class TGenericParticles : public TParticles
{
public:
	// m_uiFlags: the buffers the particles have (TGenericParticlesSettings::m_uiBuffers) and the owner's flags
	enum FLAGS : TUINT
	{
		FLAGS_POSITION          = 1 << 0,
		FLAGS_VELOCITY          = 1 << 1,
		FLAGS_LIFE              = 1 << 4,  // m_afLife and m_afLifeLostPerSec
		FLAGS_MASS              = 1 << 6,
		FLAGS_SIZE              = 1 << 7,
		FLAGS_UVINDEX           = 1 << 8,
		FLAGS_COLOUR            = 1 << 9,
		FLAGS_FORWARD           = 1 << 10, // m_avForward (ribbons with a forward direction)
		FLAGS_KEEPALIVE         = 1 << 28, // Update keeps the particles while they have no alive particle
		FLAGS_TRANSWITHXFORM    = 1 << 29, // the particles move with the transform (translation)
		FLAGS_TRANSROTWITHXFORM = 1 << 30, // the particles move with the transform (translation and rotation)
		FLAGS_REEMIT            = 1u << 31, // dead particles are emitted again
	};

	using t_FinishedCallback = void ( * )( TGenericParticles* a_pParticles, void* a_pUserData );
	using t_GetSizeFn        = TFLOAT( __stdcall* )( TGenericParticles* a_pParticles, TINT a_iParticle );

	static TCONSTEXPR_VAR TINT MAX_ROTATIONS = 16;

public:
	TGenericParticles();
	~TGenericParticles();

	//-----------------------------------------------------------------------------
	// TParticles
	//-----------------------------------------------------------------------------
	virtual void  Initialise() OVERRIDE;                              // +0x04
	virtual void  Deinitialise() OVERRIDE;                            // +0x08
	virtual TINT  GetVertsPerNumParticles( TINT a_iNumParticles ) OVERRIDE; // +0x0C
	virtual void  SetNumRenderingParticles( TINT a_iNumParticles ) OVERRIDE; // +0x14
	virtual void  SetNumParticles( TINT a_iNumParticles ) OVERRIDE;   // +0x18
	virtual void  PreUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                              // +0x20
	virtual TBOOL Update( TFLOAT a_fDeltaTime ) OVERRIDE;             // +0x24
	virtual void  Render() OVERRIDE;                                  // +0x28

	void SetSettings( TGenericParticlesSettings* a_pSettings );
	void SetTransform( const TMatrix44& a_rTransform );

	// Initialises the particles from a_iStart to a_iEnd (a_fTimeOffset: the share of the delta time they already
	// lived, a_fBlend < 0: they're spread between the previous and the current transform)
	void InitParticles( TINT a_iStart, TINT a_iEnd, TFLOAT a_fTimeOffset, TBOOL a_bInitLife, TFLOAT a_fBlend );

	void InitRotations();
	void UpdateRotations( TFLOAT a_fDeltaTime );
	void UpdateLighting();

	// $deBlob: FUNCTION 007187a0
	TBOOL HasFlag( TUINT a_uiFlag ) const { return ( m_uiFlags & a_uiFlag ) != 0; }

	// $deBlob: FUNCTION 00720180
	void SetFlag( TUINT a_uiFlag ) { m_uiFlags |= a_uiFlag; }

	// $deBlob: FUNCTION 00720160
	void SetFinishedCallback( t_FinishedCallback a_fnCallback, void* a_pUserData )
	{
		m_fnFinished        = a_fnCallback;
		m_pFinishedUserData = a_pUserData;
	}

	TGenericParticlesSettings* GetSettings() const { return m_pSettings; }
	TMatrix44&                 GetTransform() { return m_oTransform; }

	//-----------------------------------------------------------------------------
	// The vertex writers (TParticles::t_SetVertsFn), picked by Initialise from the settings
	//-----------------------------------------------------------------------------
	static void __stdcall SetPosition( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetPositionSizeBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetPositionSizeBlendRotate( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetPositionConstantSizeBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetPositionSpark( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetPositionSparkQuad( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetPositionRibbon( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles, const TVector4* a_pUp );
	static void __stdcall SetConstantUV( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetConstantUVRibbon( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetUVFromUVIndexRotate( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetUVFromUVIndex( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetUVFromUVIndexRibbon( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetUVFromUVIndexSparks( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetConstantColour( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetColourFromBuffer( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetColourFromKeyframe( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetColourFromBufferWithLighting( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetColourFromKeyframeWithLighting( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetConstantColourBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );
	static void __stdcall SetColourBlend( TParticleMesh* a_pMesh, TINT a_iVertIndex, TINT a_iParticle, TParticles* a_pParticles );

	// The ribbons' index writer (and the vertices: RENDERSTYLE_RIBBON renders with RenderVertsRibbon)
	static TINT __stdcall RenderRibbonIndices( TParticleMesh* a_pMesh, TINT a_iNumParticles, TParticles* a_pParticles );

	// The sizes (m_fnGetSize)
	static TFLOAT __stdcall GetSizeFixed( TGenericParticles* a_pParticles, TINT a_iParticle );
	static TFLOAT __stdcall GetSizeKeyframe( TGenericParticles* a_pParticles, TINT a_iParticle );

private:
	void RenderVertsRibbon( t_SetVertsFn a_fnSetPosition, t_SetVertsFn a_fnSetUV, t_SetVertsFn a_fnSetColour, t_SetIndicesFn a_fnSetIndices, TParticles* a_pParticles );
	void RenderVertsMesh();

public:
	// The quad corners (RENDERSTYLE_QUAD: the particle system's camera facing offsets, set by Render)
	static TVector4 s_vBLOffset;
	static TVector4 s_vTLOffset;
	static TVector4 s_vBROffset;
	static TVector4 s_vTROffset;

	// The corners of the rotated quads (s_iNumRotations rotations of 4 corners, built by UpdateRotations)
	static TVector4 s_avQuadRotations[ MAX_ROTATIONS * 4 ];
	static TINT     s_iNumRotations;

	// The lighting (UpdateLighting)
	static TINT s_iLightR;
	static TINT s_iLightG;
	static TINT s_iLightB;

	// The constants of the settings being rendered (Render)
	static TUINT32  s_uiConstCol0;
	static TUINT32  s_uiConstCol1;
	static TFLOAT   s_fConstSize0;
	static TFLOAT   s_fConstSize1;
	static TFLOAT   s_fYoungestLife;     // (ribbons)
	static TFLOAT   s_fOldestLife;       // (ribbons: 1 / the life range)
	static TFLOAT   s_fRibbonOldestLife; // (ribbons: the oldest life)
	static TVector4 s_vRibbonUp;         // (ribbons: the up direction)

public:
	TParticleV4Buffer          m_avPosition;        // 0x028
	TParticleV4Buffer          m_avVelocity;        // 0x038
	TParticleV4Buffer          m_avForward;         // 0x048
	TParticleFloatBuffer       m_afLife;            // 0x058 (0: born, 1: dead)
	TParticleFloatBuffer       m_afLifeLostPerSec;  // 0x068
	TParticleFloatBuffer       m_afMass;            // 0x078
	TParticleFloatBuffer       m_afSize;            // 0x088
	TParticleUIntBuffer        m_aiUVIndex;         // 0x098
	TParticleUIntBuffer        m_auiColour;         // 0x0A8
	TParticleUIntBuffer        m_apCurColourKey;    // 0x0B8 (TParticleKeyframe*)
	TParticleUIntBuffer        m_apCurSizeKey;      // 0x0C8 (TParticleKeyframe*)
	TParticleUCharBuffer       m_auiEventIndex;     // 0x0D8 (the next event key)
	TVector4                   m_vPrevTranslation;  // 0x0E8
	TQuaternion                m_qPrevRotation;     // 0x0F8
	TMatrix44                  m_oTransform;        // 0x108
	TVector3                   m_vBillboardUp;      // 0x148
	TVector3                   m_vBillboardForward; // 0x154
	t_FinishedCallback         m_fnFinished;        // 0x160
	void*                      m_pFinishedUserData; // 0x164
	TFLOAT                     m_afRotations[ MAX_ROTATIONS ]; // 0x168
	t_SetVertsFn               m_fnSetPosition;     // 0x1A8
	t_SetVertsFn               m_fnSetUV;           // 0x1AC
	t_SetVertsFn               m_fnSetColour;       // 0x1B0
	t_SetIndicesFn             m_fnSetIndices;      // 0x1B4
	t_GetSizeFn                m_fnGetSize;         // 0x1B8
	TUINT                      m_uiFlags;           // 0x1BC
	TFLOAT                     m_fTime;             // 0x1C0
	TINT                       m_iNumEmitting;      // 0x1C4 (SetNumRenderingParticles, until PreUpdate)
	TINT                       m_iNumEmitted;       // 0x1C8
	TGenericParticlesSettings* m_pSettings;         // 0x1CC
	TParticleKeyframeState     m_aStates[ 4 ];      // 0x1D0
	TFLOAT                     m_afConstantUVs[ 8 ]; // 0x210 (SetConstantUV: the corners' UVs)
};

TSTATICASSERT( sizeof( TGenericParticles ) == 0x230 );

TOSHI_NAMESPACE_END
