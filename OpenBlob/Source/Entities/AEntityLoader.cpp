#include "pch.h"
#include "AEntityLoader.h"
#include "AAssetStreaming.h"
#include "ACurrentLevelInformation.h"
#include "AEntityFactory.h"
#include "AEntityHash.h"
#include "GameController/AGameStatus.h"
#include "Challenges/AChallengeManager.h"
#include "Buildings/ABuildingManager.h"

#include <Toshi/T2String8.h>
#include <Math/TQuaternion.h>

#include <Toshi/THPTimer.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00793450
AEntityLoader::AEntityLoader()
{
	m_pTRB       = TNULL;
	m_eState     = STATE_Loading;
	m_pParamList = TNULL;
	m_iCurEnt    = 0;
	m_itEntity   = TNULL;
	m_pTRB       = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_Entities );
}

// $deBlob: FUNCTION 00793960
TBOOL AEntityLoader::SkipInLevelType( const EntityData& a_rEntityData )
{
	TINT iSkipInLevelType = 0;

	if ( !a_rEntityData.GetValue( iSkipInLevelType, "SkipInLevelType" ) )
		return TFALSE;

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT         iGameType   = pGameStatus->GetMultiPlayerGameType();

	if ( iGameType < 0 )
	{
		switch ( pGameStatus->GetLevelMode() )
		{
			case -1: return ( iSkipInLevelType & 0x80 ) != 0;
			case 0: return ( iSkipInLevelType & 0x100 ) != 0;
			case 1: return ( iSkipInLevelType & 0x10 ) != 0;
			case 2: return ( iSkipInLevelType & 0x20 ) != 0;
			case 3: return ( iSkipInLevelType & 0x40 ) != 0;
			default: return TFALSE;
		}
	}

	switch ( iGameType )
	{
		case 0: return ( iSkipInLevelType & 2 ) != 0;
		case 1: return ( iSkipInLevelType & 8 ) != 0;
		case 2: return ( iSkipInLevelType & 1 ) != 0;
		case 3: return ( iSkipInLevelType & 4 ) != 0;
		default: return TFALSE;
	}
}

// $deBlob: FUNCTION 007934a0
TBOOL AEntityLoader::Update( TFLOAT a_fDeltaTime )
{
	switch ( m_eState )
	{
		case STATE_Loading:
			m_eState     = STATE_Creating;
			m_pParamList = TSTATICCAST( ParamList, m_pTRB->GetSymbolAddress( "EntitiesMain" ) );

			if ( m_pParamList->m_iEntityCount == 0 )
				m_eState = STATE_PostCreate;

			break;
		case STATE_Creating:
		{
			THPTimer oTimer;
			oTimer.Reset();
			oTimer.Update();

			TFLOAT          fTime    = 0.0f;
			AEntityFactory* pFactory = AEntityFactory::GetSingleton();

			do
			{
				if ( AAssetStreaming::GetSingleton()->HasActiveJobs() )
					break;

				TASSERT( m_iCurEnt < m_pParamList->m_iEntityCount );
				EntityData* pEntityData = &m_pParamList->m_pEntities[ m_iCurEnt++ ];

				if ( !SkipInLevelType( *pEntityData ) )
				{
					ABaseEntity* pEntity = pFactory->CreateEntity( pEntityData->GetTypeName() );

#ifdef DEBLOB_COMMUNITY_PATCH
					// NOTE: temporary scaffolding: the factory returns TNULL for the types
					// whose classes aren't ported
					if ( pEntity != TNULL )
#endif
					{
						pEntity->SetID( pEntityData->m_iID );
						pEntity->ProcessParams( *pEntityData );
						pEntity->SetBBox( *pEntityData->m_pBBox );

						const TMatrix44* pMatrix = pEntityData->m_pMatrix;
						TMatrix44        oMatrix;

						if ( !pMatrix->IsOrthonormal() )
						{
							TQuaternion oQuat;
							TMatrix44::MatToQuat( oQuat, *pMatrix );
							oQuat /= TMath::Sqrt( oQuat.x * oQuat.x + oQuat.y * oQuat.y + oQuat.z * oQuat.z + oQuat.w * oQuat.w );
							oMatrix.SetFromQuaternion( oQuat );
							pMatrix = &oMatrix;
						}

						pEntity->SetMatrix( *pMatrix );
						pEntity->SetPosition( pEntityData->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION ) );
						pEntity->SyncBodyTransform();

						const TCHAR* szIDName = TNULL;
						if ( pEntityData->GetValue( szIDName, "IDName" ) && T2String8::Compare( szIDName, "auto_cam" ) == 0 )
							pEntity->SetName( "auto_cam" );

						AEntityHash::GetSingleton()->Add( *pEntity, TFALSE );
						pEntity->PreRenderUpdate();
					}
				}

				oTimer.Update();
				fTime += oTimer.GetDelta();

				if ( m_iCurEnt >= m_pParamList->m_iEntityCount )
				{
					m_eState = STATE_PostCreate;
					break;
				}
			} while ( fTime < 0.005f );

			break;
		}
		case STATE_PostCreate:
			ABuildingManager::GetSingleton()->SetupPaintableBlocks();
			AEntityFactory::GetSingleton()->CreatePools();
			m_itEntity = AEntityFactory::GetSingleton()->GetEntities().Begin();
			m_eState   = STATE_PostCreateEntities;
			break;
		case STATE_LoadChallenges:
		{
			// The challenges (only their cameras off in multiplayer and the other modes)
			AGameStatus* pGameStatus = AGameStatus::GetSingleton();
			TINT         eLevelMode  = pGameStatus->GetLevelMode();

			if ( pGameStatus->GetLevelData().m_bMultiplayer || ( eLevelMode != 0 && eLevelMode != -1 && eLevelMode != 3 ) )
				AChallengeManager::GetSingleton()->DisableAllChallengeCameras();
			else
				AChallengeManager::GetSingleton()->LoadChallenges();

			m_eState = STATE_Done;
			return TTRUE;
		}
		case STATE_PostCreateEntities:
		{
			THPTimer oTimer;
			oTimer.Reset();
			oTimer.Update();

			TFLOAT fTime = 0.0f;

			do
			{
				AEntityFactory* pFactory = AEntityFactory::GetSingleton();

				if ( m_itEntity == pFactory->GetEntities().End() )
				{
					pFactory->InitPooledEntities();
					m_eState = STATE_LoadChallenges;
					break;
				}

				ABaseEntity* pEntity = m_itEntity++;
				pEntity->PostCreate();

				if ( pEntity->IsFlagged( ABaseEntity::FLAG_Hashed ) )
					AEntityHash::GetSingleton()->Add( *pEntity, TFALSE );

				oTimer.Update();
				fTime += oTimer.GetDelta();
			} while ( fTime < 0.005f );

			break;
		}
		case STATE_Done:
			ABuildingManager::UncountLandmarkCivs();
			return TFALSE;
	}

	return TTRUE;
}
