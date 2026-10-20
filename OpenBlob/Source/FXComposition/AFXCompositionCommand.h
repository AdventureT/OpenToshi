#pragma once
#include "Console/AConsoleVar.h"

class AFXCompositionInstance;

//-----------------------------------------------------------------------------
// Purpose: a command of an FX composition, data in the level's FX composition TRB. AFXCompositionInstance::Update
// executes the commands whose start time has passed.
//-----------------------------------------------------------------------------
class AFXCompositionCommand
{
public:
	enum TYPE : TINT16
	{
		TYPE_SHOW                    = 0,
		TYPE_HIDE                    = 1,
		TYPE_SCALE                   = 2,
		TYPE_SCALEX                  = 3,
		TYPE_SCALEY                  = 4,
		TYPE_SCALEZ                  = 5,
		TYPE_TRANSLATE               = 6,
		TYPE_TRANSLATEX              = 7,
		TYPE_TRANSLATEY              = 8,
		TYPE_TRANSLATEZ              = 9,
		TYPE_ROTATEX                 = 10,
		TYPE_ROTATEY                 = 11,
		TYPE_ROTATEZ                 = 12,
		TYPE_ALPHA                   = 13,
		TYPE_TRIGGERPARTICLE         = 14,
		TYPE_TRIGGERPARTICLEBONE     = 15,
		TYPE_TRIGGERPARTICLEND       = 16,
		TYPE_TRIGGERPARTICLBONEEND   = 17,
		TYPE_UPDATETRANSFORM         = 18,
		TYPE_STARTLOOP               = 19,
		TYPE_ENDLOOP                 = 20,
		TYPE_ROTATEXLOOP             = 21,
		TYPE_ROTATEYLOOP             = 22,
		TYPE_ROTATEZLOOP             = 23,
		TYPE_TRIGGERPARTICLELOOP     = 24,
		TYPE_TRIGGERPARTICLEBONELOOP = 25,
		TYPE_STOPPARTICLELOOP        = 26,
		TYPE_SCALENONUNIFORM         = 27,
		TYPE_SETROTATIONX            = 28,
		TYPE_SETROTATIONY            = 29,
		TYPE_SETROTATIONZ            = 30,
	};

	enum FLAGS : TUINT8
	{
		FLAGS_SMOOTH = 1, // cosine blend
	};

	// SHOW, HIDE
	struct ModelParams
	{
		TINT m_iModel; // 0x08
	};

	// SCALE*, TRANSLATEX/Y/Z, ROTATE*, ALPHA
	struct BlendParams
	{
		TINT   m_iModel;    // 0x08
		TFLOAT m_fFrom;     // 0x0C
		TFLOAT m_fTo;       // 0x10
		TFLOAT m_fDuration; // 0x14
	};

	// TRANSLATE (local axes), SCALENONUNIFORM
	struct BlendVecParams
	{
		TINT   m_iModel;      // 0x08
		TFLOAT m_afFrom[ 3 ]; // 0x0C
		TFLOAT m_afTo[ 3 ];   // 0x18
		TFLOAT m_fDuration;   // 0x24
	};

	// TRIGGERPARTICLE, TRIGGERPARTICLELOOP, STOPPARTICLELOOP
	struct ParticleParams
	{
		TINT m_iParticle; // 0x08
	};

	// TRIGGERPARTICLEBONE, TRIGGERPARTICLEBONELOOP
	struct ParticleBoneParams
	{
		TINT         m_iModel;    // 0x08
		const TCHAR* m_szBone;    // 0x0C
		TINT         m_iParticle; // 0x10
	};

	// TRIGGERPARTICLEND
	struct ParticleEndParams
	{
		TINT   m_iParticle; // 0x08
		TFLOAT m_fDuration; // 0x0C
	};

	// TRIGGERPARTICLBONEEND
	struct ParticleBoneEndParams
	{
		TINT         m_iModel;    // 0x08
		const TCHAR* m_szBone;    // 0x0C
		TINT         m_iParticle; // 0x10
		TFLOAT       m_fDuration; // 0x14
	};

	// UPDATETRANSFORM
	struct InitTransformParams
	{
		TINT   m_iModel;    // 0x08
		TFLOAT m_fDuration; // 0x0C
	};

	// ROTATE*LOOP
	struct SpinParams
	{
		TINT   m_iModel; // 0x08
		TFLOAT m_fSpeed; // 0x0C revolutions per second
	};

	// SETROTATION*: a random angle
	struct SetRotationsParams
	{
		TINT   m_iModel; // 0x08
		TFLOAT m_fMin;   // 0x0C
		TFLOAT m_fMax;   // 0x10
	};

public:
	// Executes the command at a_fTime (time since the instance was triggered), returns TTRUE when it's done
	TBOOL Execute( TFLOAT a_fTime, TFLOAT a_fDeltaTime, AFXCompositionInstance& a_oInstance );

	// The particle system slot used by a TRIGGERPARTICLEND/TRIGGERPARTICLBONEEND/TRIGGERPARTICLE(BONE) command, else -1
	TINT GetParticleId();

	// TTRUE for the ROTATE*LOOP and TRIGGERPARTICLE*LOOP commands, which keep running
	TBOOL IsPersistentCommand();

	// a_fPercent (0..1) eased with a cosine if FLAGS_SMOOTH is set
	TFLOAT CalculateBlendValue( TFLOAT a_fPercent );

public:
	TFLOAT m_fTime;   // 0x00 start time
	TYPE   m_eType;   // 0x04
	TUINT8 m_uiFlags; // 0x06

	union
	{
		ModelParams           m_oShowParams;
		ModelParams           m_oHideParams;
		BlendParams           m_oScaleParams;
		BlendVecParams        m_oTranslateParams;
		BlendParams           m_oTranslateAxisParams;
		BlendParams           m_oRotateParams;
		BlendParams           m_oAlphaParams;
		ParticleParams        m_oTriggerParticleParams;
		ParticleBoneParams    m_oTriggerParticleBoneParams;
		ParticleEndParams     m_oTriggerParticleEndParams;
		ParticleBoneEndParams m_oTriggerParticleBoneEndParams;
		InitTransformParams   m_oInitTransformParams;
		SpinParams            m_oSpinParams;
		ParticleParams        m_oStopParticleLoopParams;
		BlendVecParams        m_oScaleNonUniformParams;
		SetRotationsParams    m_oSetRotationsParams;
	}; // 0x08
};
