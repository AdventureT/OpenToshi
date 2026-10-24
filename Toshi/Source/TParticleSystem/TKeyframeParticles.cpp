#include "ToshiPCH.h"
#include "TParticleKeyframe.h"
#include "TGenericParticles.h"
#include "TGenericParticlesSettings.h"
#include "TParticleSystem.h"
#include "TParticleEffect.h"
#include "TParticleEffectInstance.h"
#include "TParticleEffectManager.h"
#include "Math/TMathInline.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// TParticleKeyframeEventRandomiseVelDirs's random (0x009b3558, constructed by the file's static initialiser)
// $deBlob: FUNCTION 0053ffa0
static TRandom s_oRandomiseVelDirsRandom;

//-----------------------------------------------------------------------------
// TKeyframeParticles
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0071ff10
void TKeyframeParticles::Link( TParticleKeyframe* a_pKey )
{
	a_pKey->m_fnStart          = Keyframe_DefaultStart;
	a_pKey->m_fnUpdate         = Update;
	a_pKey->m_fnUpdateParticle = Keyframe_DefaultUpdateParticle;
}

// $deBlob: FUNCTION 0071ff50
void __stdcall TKeyframeParticles::Update( TParticleKeyframe* a_pKey, TFLOAT a_fDeltaTime, TFLOAT a_fT, TParticleKeyframeState* a_pState )
{
	TParticleKeyframe::ParticleSystemData* pData   = a_pKey->GetData<TParticleKeyframe::ParticleSystemData>();
	TParticleEffectInstance::System*       pSystem = TREINTERPRETCAST( TParticleEffectInstance::System*, a_pState );
	TFLOAT                                 fFrom   = 0.0f;

	if ( pSystem->m_pParticles == TNULL )
	{
		if ( 0.0f < pData->m_fNumParticles )
		{
			TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, TParticleSystem::GetSingleton()->CreateParticles( TParticleSystem::PARTICLES_GENERIC ) );

			if ( pParticles == TNULL )
			{
				// No free particles: steal some
				TParticleSystem::GetSingleton()->GetEffectManager()->StopStealableEffectInstance();
			}
			else
			{
				TParticleEffectSystem* pEffectSystem = TParticleEffectInstance::s_pActiveEffectSys;
				pParticles->SetSettings( pData->m_pSettings );
				pParticles->m_uiFlags |= TGenericParticles::FLAGS_REEMIT | TGenericParticles::FLAGS_KEEPALIVE;

				if ( pEffectSystem->HasFlag( TParticleEffectSystem::FLAGS_TRANSWITHXFORM ) )
					pParticles->m_uiFlags |= TGenericParticles::FLAGS_TRANSWITHXFORM;
				else if ( pEffectSystem->HasFlag( TParticleEffectSystem::FLAGS_TRANSROTWITHXFORM ) )
					pParticles->m_uiFlags |= TGenericParticles::FLAGS_TRANSROTWITHXFORM;

				pParticles->SetTransform( TParticleEffectInstance::s_mFinalTransform );
				pParticles->m_pFinishedUserData = TParticleEffectInstance::s_pActiveEffectInstance;
				pParticles->m_fnFinished        = TREINTERPRETCAST( TGenericParticles::t_FinishedCallback, TParticleEffectInstance::GenericParticleFinished );
				pParticles->Initialise();
				pSystem->m_pParticles = pParticles;
			}
		}

		if ( pSystem->m_pParticles == TNULL )
			return;
	}

	if ( pData->m_pPrevKey != TNULL )
		fFrom = a_pKey->m_pPrev->GetData<TParticleKeyframe::ParticleSystemData>()->m_fNumParticles;

	TMath::Clip( a_fT, 0.0f, 1.0f );
	TFLOAT fNumParticles = TMath::LERP( fFrom, pData->m_fNumParticles, a_fT );
	TINT16 iNumParticles = TMath::Int32ToInt16( TINT( ( TFLOAT( pData->m_pSettings->m_iNumParticles ) + 0.9f ) * fNumParticles ) );
	TINT16 iNum          = iNumParticles;

	if ( 0.0f < pData->m_fNumParticles )
	{
		// At least one particle while the key has some
		TINT16 iOne = 1;
		iNum        = TMath::Max( iOne, iNumParticles );
	}

	TGenericParticles* pParticles = TSTATICCAST( TGenericParticles, pSystem->m_pParticles );
	pParticles->SetTransform( TParticleEffectInstance::s_mFinalTransform );
	pParticles->SetNumParticles( TUINT16( iNum ) );
}

//-----------------------------------------------------------------------------
// TParticleKeyframeEventReverseVelDirs
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00720190
void TParticleKeyframeEventReverseVelDirs::Link( TParticleKeyframe* a_pKey )
{
	a_pKey->m_fnStart          = Keyframe_DefaultStart;
	a_pKey->m_fnUpdate         = Keyframe_DefaultUpdate;
	a_pKey->m_fnUpdateParticle = UpdateParticle;
}

// $deBlob: FUNCTION 007201c0
void __stdcall TParticleKeyframeEventReverseVelDirs::UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle )
{
	TVector4& rVelocity = a_pParticles->m_avVelocity[ a_iParticle ];
	rVelocity.x         = -rVelocity.x;
	rVelocity.y         = -rVelocity.y;
	rVelocity.z         = -rVelocity.z;
}

//-----------------------------------------------------------------------------
// TParticleKeyframeEventRandomiseVelDirs
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00720220
void TParticleKeyframeEventRandomiseVelDirs::Link( TParticleKeyframe* a_pKey )
{
	a_pKey->m_fnStart          = Keyframe_DefaultStart;
	a_pKey->m_fnUpdate         = Keyframe_DefaultUpdate;
	a_pKey->m_fnUpdateParticle = UpdateParticle;
}

// NOTE: the directions are random, not normalised, and the speed isn't kept
// $deBlob: FUNCTION 00720250
void __stdcall TParticleKeyframeEventRandomiseVelDirs::UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle )
{
	TFLOAT fX                                   = s_oRandomiseVelDirsRandom.GetFloat2();
	a_pParticles->m_avVelocity[ a_iParticle ].x = fX;
	TFLOAT fY                                   = s_oRandomiseVelDirsRandom.GetFloat2();
	a_pParticles->m_avVelocity[ a_iParticle ].y = fY;
	TFLOAT fZ                                   = s_oRandomiseVelDirsRandom.GetFloat2();
	a_pParticles->m_avVelocity[ a_iParticle ].z = fZ;
}

TOSHI_NAMESPACE_END
