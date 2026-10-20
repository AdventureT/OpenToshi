#pragma once
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

class ATRBLoaderJob;
class ASky;

namespace Toshi
{
class TTRB;
class TParticleEffect;
class TParticleEffectInstance;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: two colours of a sky layer (the colours of the sky gradient or of a cloud model), blended by the
// environment settings
//-----------------------------------------------------------------------------
struct AEnvFXColours
{
	void Lerp( const AEnvFXColours& a_rTarget, TFLOAT a_fBlend );

	Toshi::TVector4 m_vColour1; // 0x00 top of the sky / the cloud's first colour
	Toshi::TVector4 m_vUnk10;   // 0x10
	Toshi::TVector4 m_vColour2; // 0x20 bottom of the sky / the cloud's second colour
	Toshi::TVector4 m_vUnk30;   // 0x30
};                              // 0x40

//-----------------------------------------------------------------------------
// Purpose: the environment effects of a level: the settings of data/envfx/*.trb (sky and cloud colours, the
// background/fog colour, fog and LOD distances, weather particle effects), blended between two settings, and the sky
// (ASky).
//-----------------------------------------------------------------------------
class AEnvironmentFX : public Toshi::TSingleton<AEnvironmentFX>
{
public:
	// One "settings" entry of the TRB (0x288 bytes); colours are 0-255 in the file (scaled by InitSettings)
	struct Settings
	{
		Toshi::TVector4         m_avUnk000[ 5 ];               // 0x000
		Toshi::TVector4         m_vBackgroundColour;           // 0x050 the clear and fog colour
		Toshi::TVector4         m_avUnk060[ 11 ];              // 0x060 (0x090-0x100 are colours)
		AEnvFXColours           m_oSkyColours;                 // 0x110
		AEnvFXColours           m_aCloudColours[ 3 ];          // 0x150
		Toshi::TVector4         m_vLightingScale5;             // 0x210 TWorldShaderHAL PS c5 / ASkinShaderHAL tev reg 0
		Toshi::TVector4         m_vLightingScale6;             // 0x220 TWorldShaderHAL PS c6 / ASkinShaderHAL tev reg 1
		TFLOAT                  m_fUnk230;                     // 0x230
		TFLOAT                  m_fUnk234;                     // 0x234
		TFLOAT                  m_fFogStart;                   // 0x238
		TFLOAT                  m_fFogEnd;                     // 0x23C
		TFLOAT                  m_afInstanceLODDistances[ 4 ]; // 0x240 (0x244/0x248 are also the entity LOD distances)
		TFLOAT                  m_afUnk250[ 5 ];               // 0x250
		TFLOAT                  m_fCloudSpeed;                 // 0x264
		TFLOAT                  m_fUnk268;                     // 0x268
		TFLOAT                  m_fUnk26C;                     // 0x26C
		TFLOAT                  m_fUnk270;                     // 0x270
		const TCHAR*            m_szEffect;                    // 0x274 the weather particle effect
		Toshi::TParticleEffect* m_pEffect;                     // 0x278
		Toshi::TParticleEffect* m_pEffectCopy;                 // 0x27C its particle counts are scaled by the blend
		TUINT32                 m_uiUnk280;                    // 0x280
		TFLOAT                  m_fUnk284;                     // 0x284
	};                                                         // 0x288

	// The "skyspace" symbol: the sky goes to space (the clouds descend and fade out, the star models show)
	struct SkySpace
	{
		TFLOAT m_fDuration;         // 0x00
		TFLOAT m_fCloudSpeed;       // 0x04 the start speed of the clouds' descent
		TFLOAT m_fCloudDescendTime; // 0x08
		TFLOAT m_fStarsHeight;      // 0x0C the star models start that much lower
		TFLOAT m_fStarsSpeed;       // 0x10
		TFLOAT m_fStarsTime;        // 0x14
	};

public:
	AEnvironmentFX();
	~AEnvironmentFX();

	void  CreateSky();
	void  Reset();
	void  LoadSettings( const TCHAR* a_szFileName, TBOOL a_bDataBlock );
	TBOOL UpdateLoading();
	void  Update( TFLOAT a_fDeltaTime );
	void  Render( void* a_pCamera );
	void  SetBlend( TFLOAT a_fBlend, TFLOAT a_fDuration, TBOOL a_bForce );
	void  StartSkySpace();

	// The lighting scale between the current PS c5 and c6 scales
	static Toshi::TVector4* GetLightingScale( Toshi::TVector4* a_pOut, TFLOAT a_fBlend );

	ASky* GetSky() const { return m_pSky; }
	void  SetSkyEnabled( TBOOL a_bEnabled ) { m_bSkyEnabled = a_bEnabled; }

private:
	void  UnloadSettings();
	TBOOL OnSettingsLoaded();
	TINT  InitSettings( Toshi::TTRB* a_pTRB );
	// The number of "settings" entries
	TINT CountSettings( Toshi::TTRB* a_pTRB );
	// Back to the first settings without a blend
	void ResetBlend();
	void ApplySettings();

	static TBOOL ReleaseSettings( Toshi::TTRB* a_pTRB );
	static void  BlendSettings( Settings* a_pOut, const Settings* a_pFrom, const Settings* a_pTo, TFLOAT a_fBlend );

private:
	TBOOL                           m_bSkyEnabled;       // 0x00
	ATRBLoaderJob*                  m_pSettingsJob;      // 0x04
	Toshi::TTRB*                    m_pSettingsTRB;      // 0x08
	TBOOL                           m_bOwnsSettingsTRB;  // 0x0C streamed by the job (not a data block)
	Settings**                      m_ppSettings;        // 0x10 TNULL terminated
	TINT                            m_iNumSettings;      // 0x14
	TINT                            m_iCurrentSettings;  // 0x18
	TFLOAT                          m_fBlendFrom;        // 0x1C
	TFLOAT                          m_fBlendTo;          // 0x20
	TBOOL                           m_bBlending;         // 0x24
	TFLOAT                          m_fBlendTime;        // 0x28
	TFLOAT                          m_fBlendDuration;    // 0x2C
	Settings*                       m_pCurrentSettings;  // 0x30
	TBOOL                           m_bSkySpace;         // 0x34
	TBOOL                           m_bHasSkySpace;      // 0x35
	TFLOAT                          m_fSkySpaceDuration; // 0x38
	TFLOAT                          m_fSkySpaceTime;     // 0x3C
	ASky*                           m_pSky;              // 0x40
	Toshi::TParticleEffectInstance* m_pEffect1;          // 0x44 of the first settings
	Toshi::TParticleEffectInstance* m_pEffect2;          // 0x48 of the second settings
};                                                       // 0x4C
