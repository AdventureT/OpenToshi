#include "pch.h"
#include "AFXCompositionCommand.h"
#include "Console/AStaticCVars.h"
#include "AFXCompositionInstance.h"
#include "Animation/AModelInstance2.h"
#include "ARandom.h"

#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: retail inlines every Execute_TYPE_* function into this switch
// $deBlob: FUNCTION 005b33d0
TBOOL AFXCompositionCommand::Execute( TFLOAT a_fTime, TFLOAT a_fDeltaTime, AFXCompositionInstance& a_oInstance )
{
	AFXCompositionData* pFXData = a_oInstance.m_pFXData;

	switch ( m_eType )
	{
		case TYPE_SHOW:
			TASSERT( m_oShowParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			a_oInstance.m_apModels[ m_oShowParams.m_iModel ]->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
			return TTRUE;
		case TYPE_HIDE:
			TASSERT( m_oHideParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			a_oInstance.m_apModels[ m_oHideParams.m_iModel ]->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
			return TTRUE;
		case TYPE_SCALE:
		{
			TASSERT( m_oScaleParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			AModelInstance2* pModel = a_oInstance.m_apModels[ m_oScaleParams.m_iModel ];

			if ( a_fTime <= m_fTime + m_oScaleParams.m_fDuration )
			{
				TFLOAT fBlend = CalculateBlendValue( ( a_fTime - m_fTime ) / m_oScaleParams.m_fDuration );
				pModel->SetScale( ( m_oScaleParams.m_fTo - m_oScaleParams.m_fFrom ) * fBlend + m_oScaleParams.m_fFrom );
				return TFALSE;
			}

			pModel->SetScale( m_oScaleParams.m_fTo );
			return TTRUE;
		}
		case TYPE_SCALEX:
		case TYPE_SCALEY:
		case TYPE_SCALEZ:
		{
			TASSERT( m_oScaleParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			AModelInstance2* pModel = a_oInstance.m_apModels[ m_oScaleParams.m_iModel ];
			TVector4         vScale = pModel->GetScale();
			TFLOAT&          rValue = ( m_eType == TYPE_SCALEX ) ? vScale.x : ( m_eType == TYPE_SCALEY ) ? vScale.y :
			                                                                                               vScale.z;

			if ( m_fTime + m_oScaleParams.m_fDuration < a_fTime )
			{
				rValue = m_oScaleParams.m_fTo;
				pModel->SetScale( vScale );
				return TTRUE;
			}

			TFLOAT fBlend = CalculateBlendValue( ( a_fTime - m_fTime ) / m_oScaleParams.m_fDuration );
			rValue        = ( m_oScaleParams.m_fTo - m_oScaleParams.m_fFrom ) * fBlend + m_oScaleParams.m_fFrom;
			pModel->SetScale( vScale );
			return TFALSE;
		}
		case TYPE_TRANSLATE:
		{
			TASSERT( m_oTranslateParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			const TMatrix44& rTransform = a_oInstance.m_oTransform;
			TVector4         vPosition  = rTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
			TVector4         vOffset;
			TBOOL            bDone;
			TFLOAT           afValues[ 3 ];

			if ( a_fTime <= m_oTranslateParams.m_fDuration + m_fTime )
			{
				TFLOAT fBlend = CalculateBlendValue( ( a_fTime - m_fTime ) / m_oTranslateParams.m_fDuration );

				for ( TINT i = 0; i < 3; i++ )
					afValues[ i ] = ( m_oTranslateParams.m_afTo[ i ] - m_oTranslateParams.m_afFrom[ i ] ) * fBlend + m_oTranslateParams.m_afFrom[ i ];

				bDone = TFALSE;
			}
			else
			{
				for ( TINT i = 0; i < 3; i++ )
					afValues[ i ] = m_oTranslateParams.m_afTo[ i ];

				bDone = TTRUE;
			}

			vOffset.Multiply( rTransform.AsBasisVector4( BASISVECTOR_RIGHT ), afValues[ 0 ] );
			vPosition.Add( vOffset );
			vOffset.Multiply( rTransform.AsBasisVector4( BASISVECTOR_UP ), -afValues[ 1 ] );
			vPosition.Add( vOffset );
			vOffset.Multiply( rTransform.AsBasisVector4( BASISVECTOR_FORWARD ), afValues[ 2 ] );
			vPosition.Add( vOffset );

			a_oInstance.m_apModels[ m_oTranslateParams.m_iModel ]->SetPosition( vPosition );
			return bDone;
		}
		case TYPE_TRANSLATEX:
		case TYPE_TRANSLATEY:
		case TYPE_TRANSLATEZ:
		{
			TASSERT( m_oTranslateAxisParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			const TMatrix44& rTransform = a_oInstance.m_oTransform;
			TVector4         vPosition  = rTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
			TFLOAT           fValue;
			TBOOL            bDone;

			if ( a_fTime <= m_fTime + m_oTranslateAxisParams.m_fDuration )
			{
				TFLOAT fBlend = CalculateBlendValue( ( a_fTime - m_fTime ) / m_oTranslateAxisParams.m_fDuration );
				fValue        = ( m_oTranslateAxisParams.m_fTo - m_oTranslateAxisParams.m_fFrom ) * fBlend + m_oTranslateAxisParams.m_fFrom;
				bDone         = TFALSE;
			}
			else
			{
				fValue = m_oTranslateAxisParams.m_fTo;
				bDone  = TTRUE;
			}

			TVector4 vOffset;

			if ( m_eType == TYPE_TRANSLATEX )
				vOffset.Multiply( rTransform.AsBasisVector4( BASISVECTOR_RIGHT ), fValue );
			else if ( m_eType == TYPE_TRANSLATEY )
				vOffset.Multiply( rTransform.AsBasisVector4( BASISVECTOR_UP ), -fValue );
			else
				vOffset.Multiply( rTransform.AsBasisVector4( BASISVECTOR_FORWARD ), fValue );

			vPosition.Add( vOffset );
			a_oInstance.m_apModels[ m_oTranslateAxisParams.m_iModel ]->SetPosition( vPosition );
			return bDone;
		}
		case TYPE_ROTATEX:
		case TYPE_ROTATEY:
		case TYPE_ROTATEZ:
		{
			TASSERT( m_oRotateParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			AModelInstance2* pModel = a_oInstance.m_apModels[ m_oRotateParams.m_iModel ];

			TMatrix44 mTransform;
			pModel->GetTransform( mTransform );
			TVector4 vPosition = mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
			mTransform         = a_oInstance.m_oTransform;

			TFLOAT fAngle;
			TBOOL  bDone;

			if ( a_fTime <= m_fTime + m_oRotateParams.m_fDuration )
			{
				TFLOAT fBlend = CalculateBlendValue( ( a_fTime - m_fTime ) / m_oRotateParams.m_fDuration );
				fAngle        = ( m_oRotateParams.m_fTo - m_oRotateParams.m_fFrom ) * fBlend + m_oRotateParams.m_fFrom;
				bDone         = TFALSE;
			}
			else
			{
				fAngle = m_oRotateParams.m_fTo;
				bDone  = TTRUE;
			}

			if ( m_eType == TYPE_ROTATEX )
				mTransform.RotateX( fAngle );
			else if ( m_eType == TYPE_ROTATEY )
				mTransform.RotateY( fAngle );
			else
				mTransform.RotateZ( fAngle );

			mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vPosition;
			pModel->SetTransform( mTransform );
			return bDone;
		}
		case TYPE_ALPHA:
		{
			TASSERT( m_oAlphaParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			TModelInstance* pModelInstance = a_oInstance.m_apModels[ m_oAlphaParams.m_iModel ]->GetModelInstance();

			if ( a_fTime <= m_fTime + m_oAlphaParams.m_fDuration )
			{
				TFLOAT fBlend = CalculateBlendValue( ( a_fTime - m_fTime ) / m_oAlphaParams.m_fDuration );
				pModelInstance->SetForceFade( TTRUE );
				pModelInstance->SetFade( ( m_oAlphaParams.m_fTo - m_oAlphaParams.m_fFrom ) * fBlend + m_oAlphaParams.m_fFrom );
				return TFALSE;
			}

			pModelInstance->SetForceFade( TTRUE );
			pModelInstance->SetFade( m_oAlphaParams.m_fTo );
			return TTRUE;
		}
		case TYPE_TRIGGERPARTICLE:
			TASSERT( m_oTriggerParticleParams.m_iParticle < a_oInstance.m_pFXData->m_pParticleSystems->m_iNumParticles );

			if ( a_oInstance.m_aParticleEffects[ m_oTriggerParticleParams.m_iParticle ].m_pSystem != TNULL )
			{
				if ( AStaticCVars::cvEnableParticles.GetBool() == TTRUE )
				{
					TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( a_oInstance.m_aParticleEffects[ m_oTriggerParticleParams.m_iParticle ].m_pSystem );

					if ( pEffect != TNULL )
					{
						pEffect->GetTransform() = a_oInstance.m_oTransform;
						return TTRUE;
					}
				}
			}
			break;
		case TYPE_TRIGGERPARTICLEBONE:
		{
			TASSERT( m_oTriggerParticleBoneParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			AModelInstance2* pModel = a_oInstance.m_apModels[ m_oTriggerParticleBoneParams.m_iModel ];
			TINT             iBone  = TINT16( pModel->FindBone( m_oTriggerParticleBoneParams.m_szBone ) );
			TASSERT( iBone >= 0 );

			if ( iBone < 0 )
				return TTRUE;

			TMatrix44 mBone;
			pModel->GetBoneTransform( iBone, mBone );
			TASSERT( m_oTriggerParticleBoneParams.m_iParticle < a_oInstance.m_pFXData->m_pParticleSystems->m_iNumParticles );

			if ( a_oInstance.m_aParticleEffects[ m_oTriggerParticleBoneParams.m_iParticle ].m_pSystem != TNULL )
			{
				if ( AStaticCVars::cvEnableParticles.GetBool() == TTRUE )
				{
					TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( a_oInstance.m_aParticleEffects[ m_oTriggerParticleBoneParams.m_iParticle ].m_pSystem );

					if ( pEffect != TNULL )
					{
						pEffect->GetTransform() = mBone;
						return TTRUE;
					}
				}
			}
			break;
		}
		case TYPE_TRIGGERPARTICLEND:
		{
			TASSERT( m_oTriggerParticleEndParams.m_iParticle < a_oInstance.m_pFXData->m_pParticleSystems->m_iNumParticles );
			AFXCompositionInstance::ParticleEffect& rEffect = a_oInstance.m_aParticleEffects[ m_oTriggerParticleEndParams.m_iParticle ];

			if ( rEffect.m_pEffect != TNULL )
			{
				if ( a_fTime <= m_oTriggerParticleEndParams.m_fDuration + m_fTime )
				{
					rEffect.m_pEffect->GetTransform() = a_oInstance.m_oTransform;
					return TFALSE;
				}

				rEffect.m_pEffect->SetFinishedCallback( TNULL, TNULL );
				rEffect.m_pEffect->Stop();
				rEffect.m_pEffect = TNULL;
				return TTRUE;
			}

			if ( rEffect.m_pSystem == TNULL )
				return TTRUE;

			{
				TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( rEffect.m_pSystem );

				if ( pEffect == TNULL )
					return TFALSE;

				pEffect->GetTransform() = a_oInstance.m_oTransform;
				pEffect->SetFinishedCallback( AFXCompositionInstance::TParticleEffectCommandEndCallback, &a_oInstance );
				rEffect.m_pEffect = pEffect;
			}
			return TFALSE;
		}
		case TYPE_TRIGGERPARTICLBONEEND:
		{
			TASSERT( m_oTriggerParticleBoneEndParams.m_iParticle < a_oInstance.m_pFXData->m_pParticleSystems->m_iNumParticles );
			AFXCompositionInstance::ParticleEffect& rEffect = a_oInstance.m_aParticleEffects[ m_oTriggerParticleBoneEndParams.m_iParticle ];
			AModelInstance2*                        pModel  = a_oInstance.m_apModels[ m_oTriggerParticleBoneEndParams.m_iModel ];

			if ( rEffect.m_pEffect != TNULL )
			{
				if ( m_oTriggerParticleBoneEndParams.m_fDuration + m_fTime < a_fTime )
				{
					rEffect.m_pEffect->SetFinishedCallback( TNULL, TNULL );
					rEffect.m_pEffect->Stop();
					rEffect.m_pEffect = TNULL;
					return TTRUE;
				}

				TINT iBone = TINT16( pModel->FindBone( m_oTriggerParticleBoneEndParams.m_szBone ) );
				TASSERT( iBone >= 0 );

				if ( iBone < 0 )
					return TTRUE;

				TMatrix44 mBone;
				pModel->GetBoneTransform( iBone, mBone );

				rEffect.m_pEffect->GetTransform() = mBone;
				return TFALSE;
			}

			TINT iBone = TINT16( pModel->FindBone( m_oTriggerParticleBoneEndParams.m_szBone ) );
			TASSERT( iBone >= 0 );

			if ( iBone < 0 )
				return TTRUE;

			TMatrix44 mBone;
			pModel->GetBoneTransform( iBone, mBone );

			if ( rEffect.m_pSystem == TNULL )
				return TTRUE;

			if ( AStaticCVars::cvEnableParticles.GetBool() == TTRUE )
			{
				TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( rEffect.m_pSystem );

				if ( pEffect == TNULL )
					return TFALSE;

				pEffect->GetTransform() = mBone;
				pEffect->SetFinishedCallback( AFXCompositionInstance::TParticleEffectCommandEndCallback, &a_oInstance );
				rEffect.m_pEffect = pEffect;
				return TFALSE;
			}
			return TTRUE;
		}
		case TYPE_UPDATETRANSFORM:
			TASSERT( m_oInitTransformParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			a_oInstance.m_apModels[ m_oInitTransformParams.m_iModel ]->SetTransform( a_oInstance.m_oTransform );
			return m_oInitTransformParams.m_fDuration + m_fTime < a_fTime;
		case TYPE_ENDLOOP:
			if ( !( a_oInstance.m_uiFlags & AFXCompositionInstance::FLAGS_STOPPING ) )
			{
				TINT iNextCommand = a_oInstance.m_iNextCommand - 1;

				for ( ; iNextCommand >= 0; iNextCommand-- )
				{
					if ( pFXData->m_apCommands[ iNextCommand ]->m_eType == TYPE_STARTLOOP )
						break;
				}

				TASSERT( iNextCommand >= 0, "FX Composer Corresponding StartLoop not found for EndLoop" );
				// NOTE: retail reads m_apCommands[ -1 ] if there's no STARTLOOP (bad data only)
				a_oInstance.m_iNextCommand = iNextCommand;
				a_oInstance.m_fTime        = pFXData->m_apCommands[ iNextCommand ]->m_fTime;
				return TTRUE;
			}
			break;
		case TYPE_ROTATEXLOOP:
		case TYPE_ROTATEYLOOP:
		case TYPE_ROTATEZLOOP:
		{
			// Replaces the other loop of this type on the same model
			for ( TINT i = 0; i < AFXCompositionInstance::MAX_RUNNING_COMMANDS; i++ )
			{
				AFXCompositionCommand* pCommand = a_oInstance.m_apRunningCommands[ i ];

				if ( pCommand != TNULL && this != pCommand && pCommand->m_eType == m_eType && pCommand->m_oSpinParams.m_iModel == m_oSpinParams.m_iModel )
				{
					a_oInstance.m_apRunningCommands[ i ] = TNULL;
					break;
				}
			}

			if ( m_oSpinParams.m_fSpeed == 0.0f )
				return TTRUE;

			AModelInstance2* pModel = a_oInstance.m_apModels[ m_oSpinParams.m_iModel ];
			TMatrix44        mTransform;
			pModel->GetTransform( mTransform );
			TFLOAT fAngle = m_oSpinParams.m_fSpeed * 6.2831855f * a_fDeltaTime;

			if ( m_eType == TYPE_ROTATEXLOOP )
				mTransform.RotateX( fAngle );
			else if ( m_eType == TYPE_ROTATEYLOOP )
				mTransform.RotateY( fAngle );
			else
				mTransform.RotateZ( fAngle );

			pModel->SetTransform( mTransform );
			return TFALSE;
		}
		case TYPE_TRIGGERPARTICLELOOP:
		{
			TASSERT( m_oTriggerParticleParams.m_iParticle < a_oInstance.m_pFXData->m_pParticleSystems->m_iNumParticles );
			AFXCompositionInstance::ParticleEffect& rEffect = a_oInstance.m_aParticleEffects[ m_oTriggerParticleParams.m_iParticle ];

			if ( rEffect.m_pEffect == TNULL && rEffect.m_pSystem != TNULL )
			{
				if ( AStaticCVars::cvEnableParticles.GetBool() == TTRUE )
				{
					TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( rEffect.m_pSystem );

					if ( pEffect != TNULL )
					{
						pEffect->GetTransform() = a_oInstance.m_oTransform;
						pEffect->SetFinishedCallback( AFXCompositionInstance::TParticleEffectCommandEndCallback, &a_oInstance );
						rEffect.m_pEffect = pEffect;
						return TTRUE;
					}
				}
			}
			break;
		}
		case TYPE_TRIGGERPARTICLEBONELOOP:
		{
			TASSERT( m_oTriggerParticleBoneParams.m_iParticle < a_oInstance.m_pFXData->m_pParticleSystems->m_iNumParticles );
			AFXCompositionInstance::ParticleEffect& rEffect = a_oInstance.m_aParticleEffects[ m_oTriggerParticleBoneParams.m_iParticle ];
			AModelInstance2*                        pModel  = a_oInstance.m_apModels[ m_oTriggerParticleBoneParams.m_iModel ];

			if ( rEffect.m_pEffect != TNULL )
			{
				TINT iBone = TINT16( pModel->FindBone( m_oTriggerParticleBoneParams.m_szBone ) );
				TASSERT( iBone >= 0 );

				if ( iBone < 0 )
					return TTRUE;

				TMatrix44 mBone;
				pModel->GetBoneTransform( iBone, mBone );

				rEffect.m_pEffect->GetTransform() = mBone;
				return TFALSE;
			}

			if ( rEffect.m_pSystem == TNULL )
				return TFALSE;

			if ( AStaticCVars::cvEnableParticles.GetBool() != TTRUE )
				return TFALSE;

			TINT iBone = TINT16( pModel->FindBone( m_oTriggerParticleBoneParams.m_szBone ) );
			TASSERT( iBone >= 0 );

			if ( iBone < 0 )
				return TTRUE;

			TMatrix44 mBone;
			pModel->GetBoneTransform( iBone, mBone );

			{
				TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( rEffect.m_pSystem );

				if ( pEffect == TNULL )
					return TFALSE;

				// NOTE: retail starts the loop at the composition's transform, not at the bone (it follows the bone from
				// the next update on)
				pEffect->GetTransform() = a_oInstance.m_oTransform;
				pEffect->SetFinishedCallback( AFXCompositionInstance::TParticleEffectCommandEndCallback, &a_oInstance );
				rEffect.m_pEffect = pEffect;
			}
			return TFALSE;
		}
		case TYPE_STOPPARTICLELOOP:
		{
			TASSERT( m_oStopParticleLoopParams.m_iParticle < a_oInstance.m_pFXData->m_pParticleSystems->m_iNumParticles );
			AFXCompositionInstance::ParticleEffect& rEffect = a_oInstance.m_aParticleEffects[ m_oStopParticleLoopParams.m_iParticle ];

			if ( rEffect.m_pEffect != TNULL )
			{
				rEffect.m_pEffect->SetFinishedCallback( TNULL, TNULL );
				rEffect.m_pEffect->Stop();
				rEffect.m_pEffect = TNULL;

				for ( TINT i = 0; i < AFXCompositionInstance::MAX_RUNNING_COMMANDS; i++ )
				{
					AFXCompositionCommand* pCommand = a_oInstance.m_apRunningCommands[ i ];

					if ( pCommand != TNULL && pCommand->m_eType == TYPE_TRIGGERPARTICLEBONELOOP && m_oStopParticleLoopParams.m_iParticle == pCommand->m_oTriggerParticleBoneParams.m_iParticle )
						a_oInstance.m_apRunningCommands[ i ] = TNULL;
				}

				return TTRUE;
			}
			break;
		}
		case TYPE_SCALENONUNIFORM:
		{
			TASSERT( m_oScaleNonUniformParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			AModelInstance2* pModel = a_oInstance.m_apModels[ m_oScaleNonUniformParams.m_iModel ];
			TVector4         vScale = pModel->GetScale();

			if ( m_oScaleNonUniformParams.m_fDuration + m_fTime < a_fTime )
			{
				vScale.x = m_oScaleNonUniformParams.m_afTo[ 0 ];
				vScale.y = m_oScaleNonUniformParams.m_afTo[ 1 ];
				vScale.z = m_oScaleNonUniformParams.m_afTo[ 2 ];
				pModel->SetScale( vScale );
				return TTRUE;
			}

			TFLOAT fBlend = CalculateBlendValue( ( a_fTime - m_fTime ) / m_oScaleNonUniformParams.m_fDuration );
			vScale.x      = ( m_oScaleNonUniformParams.m_afTo[ 0 ] - m_oScaleNonUniformParams.m_afFrom[ 0 ] ) * fBlend + m_oScaleNonUniformParams.m_afFrom[ 0 ];
			vScale.y      = ( m_oScaleNonUniformParams.m_afTo[ 1 ] - m_oScaleNonUniformParams.m_afFrom[ 1 ] ) * fBlend + m_oScaleNonUniformParams.m_afFrom[ 1 ];
			vScale.z      = ( m_oScaleNonUniformParams.m_afTo[ 2 ] - m_oScaleNonUniformParams.m_afFrom[ 2 ] ) * fBlend + m_oScaleNonUniformParams.m_afFrom[ 2 ];
			pModel->SetScale( vScale );
			return TFALSE;
		}
		case TYPE_SETROTATIONX:
		case TYPE_SETROTATIONY:
		case TYPE_SETROTATIONZ:
		{
			TASSERT( m_oSetRotationsParams.m_iModel < a_oInstance.m_pFXData->m_pModels->m_iNumModels );
			AModelInstance2* pModel = a_oInstance.m_apModels[ m_oSetRotationsParams.m_iModel ];
			TMatrix44        mTransform;
			pModel->GetTransform( mTransform );
			TFLOAT fAngle = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( m_oSetRotationsParams.m_fMin, m_oSetRotationsParams.m_fMax );

			if ( m_eType == TYPE_SETROTATIONX )
				mTransform.RotateX( fAngle );
			else if ( m_eType == TYPE_SETROTATIONY )
				mTransform.RotateY( fAngle );
			else
				mTransform.RotateZ( fAngle );

			pModel->SetTransform( mTransform );
			break;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005b4e80
TINT AFXCompositionCommand::GetParticleId()
{
	switch ( m_eType )
	{
		case TYPE_TRIGGERPARTICLE:
		case TYPE_TRIGGERPARTICLEND:
			return m_oTriggerParticleParams.m_iParticle;
		case TYPE_TRIGGERPARTICLEBONE:
		case TYPE_TRIGGERPARTICLBONEEND:
			return m_oTriggerParticleBoneParams.m_iParticle;
		default:
			return -1;
	}
}

// $deBlob: FUNCTION 005b4eb0
TBOOL AFXCompositionCommand::IsPersistentCommand()
{
	return m_eType == TYPE_ROTATEXLOOP || m_eType == TYPE_ROTATEYLOOP || m_eType == TYPE_ROTATEZLOOP || m_eType == TYPE_TRIGGERPARTICLEBONELOOP;
}

// $deBlob: FUNCTION 005b4ee0
TFLOAT AFXCompositionCommand::CalculateBlendValue( TFLOAT a_fPercent )
{
	TASSERT( a_fPercent >= 0.0f && a_fPercent <= 1.0f );

	if ( m_uiFlags & FLAGS_SMOOTH )
		return 1.0f - ( TFLOAT( cos( TFLOAT( a_fPercent * 3.1415927f ) ) ) + 1.0f ) * 0.5f;

	return a_fPercent;
}
