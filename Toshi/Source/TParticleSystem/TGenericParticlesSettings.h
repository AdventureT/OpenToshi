#pragma once
#include "Toshi/T2ResourceHelper.h"
#include "Math/TVector3.h"


TOSHI_NAMESPACE_START

class TParticleMaterial;
class TParticleInit;
class TParticleAffector;
class TParticleKeyframeTimeline;
class TGenericParticles;

// RENDERSTYLE_QUAD: the random start rotations of the quads (0x54 bytes)
struct TQuadRenderSettings
{
	void Init();

	TINT   m_iNumRotations;  // 0x00 (at most 16)
	TFLOAT m_fMinRotation;   // 0x04
	TFLOAT m_fMaxRotation;   // 0x08
	TFLOAT m_fUVRotation;    // 0x0C degrees (SetUVFromUVIndexRotate)
	TFLOAT m_afRotations[ 16 ]; // 0x10 (filled by Init)
	TINT   m_bFacing;        // 0x50 0: the quads face the camera, else they lie in the XZ plane
};

// RENDERSTYLE_BILLBOARDQUAD: the quads rotate around an axis (0xF4 bytes)
struct TBillboardQuadRenderSettings
{
	enum Flags : TUINT8
	{
		Flags_EmitterSpace = BITFLAG( 1 ), // the vectors are in the emitter's space
	};

	void Init();

	TVector3 m_vUp;          // 0x00
	TVector3 m_vForward;     // 0x0C
	TINT16   m_iNumRotations; // 0x18
	TUINT8   m_uiFlags;       // 0x1A Flags
	TUINT8   m_uiUnk1B;      // 0x1B
	TFLOAT   m_fMinX;        // 0x1C
	TFLOAT   m_fMaxX;        // 0x20
	TFLOAT   m_fMinY;        // 0x24
	TFLOAT   m_fMaxY;        // 0x28
	TFLOAT   m_fMinZ;        // 0x2C
	TFLOAT   m_fMaxZ;        // 0x30
	TFLOAT   m_afRotationsX[ 16 ]; // 0x34 (filled by Init)
	TFLOAT   m_afRotationsY[ 16 ]; // 0x74
	TFLOAT   m_afRotationsZ[ 16 ]; // 0xB4
};

// RENDERSTYLE_SPARKS (0x14 bytes)
struct TSparkRenderSettings
{
	TFLOAT m_fUnk00;       // 0x00
	TFLOAT m_fLength;      // 0x04 (forwards)
	TFLOAT m_fTailLength;  // 0x08 (backwards)
	TFLOAT m_fUnk0C;       // 0x0C
	TFLOAT m_fWidth;       // 0x10
};

// RENDERSTYLE_SPARKQUAD (8 bytes)
struct TSparkQuadRenderSettings
{
	TFLOAT m_fWidth;  // 0x00
	TFLOAT m_fLength; // 0x04
};

// RENDERSTYLE_RIBBON (0x14 bytes)
struct TRibbonRenderSettings
{
	enum RIBBONUP
	{
		RIBBONUP_EMITTERX = 1,
		RIBBONUP_EMITTERY,
		RIBBONUP_EMITTERZ,
	};

	enum RIBBONFORWARD
	{
		RIBBONFORWARD_EMITTERX = 1,
		RIBBONFORWARD_EMITTERY,
		RIBBONFORWARD_EMITTERZ,
	};

	TUINT32 m_bAttachToEmitter; // 0x00 (the youngest particle is kept at the emitter)
	TVector3 m_vUp;             // 0x04 (m_uiUpDir == 0)
	TUINT8 m_uiUpDir;       // 0x10
	TUINT8 m_uiForwardDir;  // 0x11 (0: none)
	TUINT8 m_aUnk12[ 2 ];   // 0x12
};

// A constant (colour or size) the vertices of all particles use, from start to end of the life (0x24 bytes)
struct TGenericParticlesConstant
{
	TGenericParticlesConstant( const TGenericParticlesConstant& a_rcOrig );

	TUINT32 m_uiUnk00;  // 0x00
	TUINT32 m_uiStart;  // 0x04 (a colour, or a float for the size)
	TUINT32 m_auiUnk08[ 3 ]; // 0x08
	TUINT32 m_uiEnd;    // 0x14
	TUINT32 m_auiUnk18[ 3 ]; // 0x18
};

// The settings of a generic particle system (0xA4 bytes), listed in the effect manager by name
class TGenericParticlesSettings
{
public:
	static TCONSTEXPR_VAR TUINT s_LatestVer = 0x30000;

	enum RENDERSTYLE
	{
		RENDERSTYLE_QUAD,
		RENDERSTYLE_BILLBOARDQUAD,
		RENDERSTYLE_SPARKS,
		RENDERSTYLE_SPARKQUAD,
		RENDERSTYLE_RIBBON,
		RENDERSTYLE_MESH,
	};

	// m_uiFlags
	enum FLAGS : TUINT
	{
		FLAGS_ROTATIONS = 1 << 2,  // random start rotations (TQuadRenderSettings, TBillboardQuadRenderSettings)
		FLAGS_LIGHTING  = 1 << 3,  // the colours are lit by the render context's ambient and light colours
		FLAGS_UVROTATE  = 1 << 4,  // the UV rects are rotated (TQuadRenderSettings::m_fUVRotation)
		FLAGS_CREATED   = 1u << 31,
	};

	// m_uiBuffers: which buffers the particles have (TGenericParticles::FLAGS_*)
	static TCONSTEXPR_VAR TUINT s_uiDefaultBuffers       = 0x3F3;
	static TCONSTEXPR_VAR TUINT s_uiDefaultRibbonBuffers = 0x7F3;

public:
	TGenericParticlesSettings( const TGenericParticlesSettings& a_rcOrig );
	~TGenericParticlesSettings();

	void Create();

	// Initialises the particles a_iStartParticle to a_iEndParticle
	void InitParticles( TGenericParticles* a_pParticles, TINT a_iStartParticle, TINT a_iEndParticle, TBOOL a_bInitLife );

	// The colours of the TRBs to the platform's (a no-op on PC)
	static TUINT32 ConvertColourToSKUColour( TUINT32 a_uiColour );

	// $deBlob: FUNCTION 007187c0
	TBOOL HasFlag( TUINT a_uiFlag ) const { return ( m_uiFlags & a_uiFlag ) != 0; }

	// $deBlob: FUNCTION 0071afd0
	TBOOL HasBuffer( TUINT a_uiBuffer ) const { return ( m_uiBuffers & a_uiBuffer ) != 0; }

	const TCHAR* GetName() const { return m_szName; }

public:
	TUINT                         m_uiVer;           // 0x00
	TGenericParticlesSettings*    m_pNext;           // 0x04
	TGenericParticlesSettings*    m_pPrev;           // 0x08
	TCHAR*                        m_szName;          // 0x0C (a copy: "<name><suffix>")
	const TCHAR*                  m_szMaterial;      // 0x10
	const TCHAR*                  m_szModel;         // 0x14 (RENDERSTYLE_MESH)
	TINT                          m_iNumParticles;   // 0x18 (the particles of a system key with m_fNumParticles 1)
	TFLOAT                        m_fUnk1C;          // 0x1C
	TFLOAT                        m_fLODDistance;    // 0x20
	TINT16                        m_iMinParticles;   // 0x24
	TBOOL                         m_bAveragePosition; // 0x26 the bounding sphere's centre is the particles' average position
	TBOOL                         m_bDynamicRadius;  // 0x27 the bounding sphere's radius is computed from the particles
	TFLOAT                        m_fRadius;         // 0x28
	TParticleMaterial*            m_pMaterial;       // 0x2C (resolved by Create)
	T2ModelPtr                    m_pModel;          // 0x30
	TFLOAT                        m_fLightScale;     // 0x34 (FLAGS_LIGHTING: the light colour's share)
	TUINT                         m_auiUnk38[ 2 ];   // 0x38
	TUINT                         m_uiFlags;         // 0x40
	TUINT                         m_uiBuffers;       // 0x44
	RENDERSTYLE                   m_eRenderStyle;    // 0x48
	TParticleAffector*            m_pAffectors;      // 0x4C
	TParticleInit*                m_pPositionInit;   // 0x50
	TParticleInit*                m_pVelocityInit;   // 0x54
	TParticleInit*                m_pLifeLostInit;   // 0x58
	TParticleInit*                m_pMassInit;       // 0x5C
	TParticleInit*                m_pSizeInit;       // 0x60
	TParticleInit*                m_pUVInit;         // 0x64
	TParticleInit*                m_pColourInit;     // 0x68
	TGenericParticlesConstant*    m_pConstColour;    // 0x6C
	TGenericParticlesConstant*    m_pConstSize;      // 0x70
	TQuadRenderSettings*          m_pQuadSettings;   // 0x74
	TBillboardQuadRenderSettings* m_pBillboardSettings; // 0x78
	TSparkRenderSettings*         m_pSparkSettings;  // 0x7C
	TSparkQuadRenderSettings*     m_pSparkQuadSettings; // 0x80
	TRibbonRenderSettings*        m_pRibbonSettings; // 0x84
	TParticleKeyframeTimeline*    m_pKeys88;         // 0x88
	TParticleKeyframeTimeline*    m_pColourKeys;     // 0x8C
	TParticleKeyframeTimeline*    m_pMassKeys;       // 0x90
	TParticleKeyframeTimeline*    m_pKeys94;         // 0x94
	TParticleKeyframeTimeline*    m_pSizeKeys;       // 0x98
	TParticleKeyframeTimeline*    m_pUVKeys;         // 0x9C
	TParticleKeyframeTimeline*    m_pEventKeys;      // 0xA0
};

TSTATICASSERT( sizeof( TGenericParticlesSettings ) == 0xA4 );

TOSHI_NAMESPACE_END
