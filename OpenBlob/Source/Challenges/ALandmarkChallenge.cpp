#include "pch.h"
#include "ALandmarkChallenge.h"
#include "AChallengeManager.h"
#include "AUberChallenge.h"
#include "Buildings/ABuildingManager.h"
#include "Buildings/ALandmarkInstance.h"
#include "Entities/ABaseEntity.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0059c750
ALandmarkChallenge::ALandmarkChallenge()
    : AChallenge( Type_Landmark )
{
	m_iTotalTriggers = 0;
	m_iTriggersLeft  = 0;
	m_iLandmarkType  = -1;
}

// The landmarks a landmark challenge can be at (m_iLandmarkType)
static const TCHAR* s_aszLandmarkTypes[] = {
	"lmark_radio_dish_b",
	"landmark_l4_propocentre_3",
	"landmark_museum",
	"landmark_dam_centre_unique",
	"landmark_guggentraz_01",
	"landmark_lighthouse",
	"landmark_inktanks",
	"centrifuge",
	"landmark_intelhq_130",
};

// $deBlob: FUNCTION 0059dd20
TBOOL ALandmarkChallenge::Create( TypeData* a_pData )
{
	if ( !AChallenge::Create( a_pData ) )
		return TFALSE;

	for ( TUINT32 i = 0; i < a_pData->m_uiNumLandmarks; i++ )
	{
		// The terrain instances become their landmarks
		a_pData->m_ppLandmarks[ i ] = ABuildingManager::GetLandmark( TINT( TREINTERPRETCAST( TUINTPTR, a_pData->m_ppLandmarks[ i ] ) ) );

		ALandmarkInstance* pLandmark    = TSTATICCAST( ALandmarkInstance, a_pData->m_ppLandmarks[ i ] );
		TINT               iNumTriggers = pLandmark->GetNumTriggers();
		TASSERT( iNumTriggers > 0, "Error: Landmark with no triggers linked to a landmark challenge!\n" );

		pLandmark->SetTriggersHidden( TTRUE );
		m_iTotalTriggers += TINT8( iNumTriggers );
		TASSERT( m_iTotalTriggers > 0 );

		if ( m_iLandmarkType < 0 )
		{
			for ( TINT j = 0; j < TARRAYSIZE( s_aszLandmarkTypes ); j++ )
			{
				if ( TStringManager::String8Compare( pLandmark->GetTerrainInstance()->m_szModel, s_aszLandmarkTypes[ j ] ) == 0 )
				{
					m_iLandmarkType = TINT8( j );
					break;
				}
			}
		}
	}

	m_iTriggersLeft = m_iTotalTriggers;
	return TTRUE;
}

// $deBlob: FUNCTION 0059de60
void ALandmarkChallenge::Start()
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	m_iTriggersLeft = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] );
		m_iTriggersLeft += TINT8( pLandmark->GetNumTriggersLeft() );
		pLandmark->SetTriggersHidden( TFALSE );
	}

	AChallengeManager* pManager = AChallengeManager::GetSingleton();
	TSTATICCAST( ABlobChallengeStatusHud, pManager->GetHudElement() )->OnChallengeStart( this );
	pManager->SetCounter( m_iTotalTriggers - m_iTriggersLeft, m_iTotalTriggers );

	AChallenge::Start();
}

// $deBlob: FUNCTION 0059dee0
void ALandmarkChallenge::Reset( TBOOL a_bUnk )
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] );

		if ( pLandmark->GetNumTriggersLeft() > 0 || ( pLandmark->m_uiFlags & ALandmarkInstance::Flags_WaitForLetterbox ) )
			pLandmark->SetTriggersHidden( TTRUE );

		if ( pData->m_uiFlags & Data::Flags_SubChallenge )
		{
			pLandmark->Reset();

			// MESSAGE_CHALLENGERESET to the entities of the landmark
			ALandmarkInstance::Links* pLinks = TSTATICCAST( ALandmarkInstance::Links, pLandmark->GetTerrainInstance()->m_pLinks );

			if ( pLinks != TNULL )
			{
				for ( TINT j = 0; j < pLinks->m_iNumEntities; j++ )
				{
					AEntityIOPacket oPacket = { AEntityIOPacket::MESSAGE_CHALLENGERESET, 0 };

					ABaseEntity* pEntity = ABaseEntity::FindIDEntity( pLinks->m_piEntityIDs[ j ] );

					if ( pEntity != TNULL )
						pEntity->HandleEntityIO( TNULL, oPacket );
				}
			}
		}
	}

	AChallenge::Reset( a_bUnk );
}

// $deBlob: FUNCTION 0059e010
void ALandmarkChallenge::Update( TFLOAT a_fDeltaTime )
{
	if ( m_iTriggersLeft > 0 )
		AChallenge::Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 0059e030
void ALandmarkChallenge::OnLandmarkEvent( void* a_pLandmark )
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		if ( pData->m_ppLandmarks[ i ] == a_pLandmark )
		{
			m_iTriggersLeft--;

			if ( pData->m_uiFlags & Data::Flags_SubChallenge )
			{
				AChallenge* pCurrChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

				if ( pCurrChallenge != TNULL )
				{
					TASSERT( pCurrChallenge->GetType() == AChallenge::Type_Uber );
					TSTATICCAST( AUberChallenge, pCurrChallenge )->IncrementLandmarksCompleted();
				}
			}

			break;
		}
	}

	AChallengeManager::GetSingleton()->SetCounter( m_iTotalTriggers - m_iTriggersLeft, m_iTotalTriggers );
}

// $deBlob: FUNCTION 0059e0e0
void ALandmarkChallenge::OnLandmarkEvent2( void* a_pLandmarkInstance )
{
	if ( m_iTriggersLeft <= 0 )
		AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );
}

// $deBlob: FUNCTION 0059e100
TBOOL ALandmarkChallenge::IsAlreadyCompleted()
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		if ( TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] )->GetNumTriggersLeft() > 0 )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0059e130
TINT ALandmarkChallenge::GetScore()
{
	TypeData* pData        = TSTATICCAST( TypeData, m_pData );
	TINT      iNumLeft     = 0;
	TINT      iNumTriggers = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] );
		iNumLeft += pLandmark->GetNumTriggersLeft();
		iNumTriggers += pLandmark->GetNumTriggers();
	}

	return TINT( ( TFLOAT( iNumTriggers - iNumLeft ) / TFLOAT( iNumTriggers ) ) * 100.0f );
}
