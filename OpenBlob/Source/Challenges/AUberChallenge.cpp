#include "pch.h"
#include "AUberChallenge.h"
#include "AChallengeManager.h"
#include "ARevolutionary/ARevolutionary.h"
#include "Entities/APropTriggerHelp.h"
#include "GameController/AGameStatus.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0059c890
AUberChallenge::AUberChallenge()
    : AChallenge( Type_Uber )
{
	m_pCurrSubChallenge       = TNULL;
	m_uiNumLandmarksCompleted = 0;
}

// $deBlob: FUNCTION 0059ffc0
void AUberChallenge::LinkChallenges()
{
	AChallenge::LinkChallenges();

	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	TASSERT( pData->m_iSubChallengeGUID != -1 );
	pData->m_pSubChallenge = AChallengeManager::GetSingleton()->FindChallengeByGUID( pData->m_iSubChallengeGUID );
}

// $deBlob: FUNCTION 005a0010
void AUberChallenge::Start()
{
	AChallenge::Start();

	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	TASSERT( pData->m_iSubChallengeGUID != -1 );

	// The first sub challenge starts with the time left
	pData->m_pSubChallenge->Start();
	m_pCurrSubChallenge = pData->m_pSubChallenge;
	m_pCurrSubChallenge->SetTimeLeft( m_fTimeLeft );
}

// $deBlob: FUNCTION 005a0070
void AUberChallenge::Reset( TBOOL a_bUnk )
{
	AChallenge::Reset( a_bUnk );

	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	TASSERT( pData->m_iSubChallengeGUID != -1 );

	for ( AChallenge* pChallenge = pData->m_pSubChallenge; pChallenge != TNULL; pChallenge = pChallenge->GetData()->m_ppLinkedChallenges[ 0 ] )
	{
		TASSERT( pChallenge->GetData()->m_uiNumLinkedChallenges <= 1, "Sub challenges must be a daisy chain of single links!" );
		pChallenge->Reset( a_bUnk );

		if ( pChallenge->GetData()->m_uiNumLinkedChallenges == 0 )
			break;
	}

	m_pCurrSubChallenge = TNULL;

	// The landmarks completed during the challenge don't count
	if ( a_bUnk && TINT( m_uiNumLandmarksCompleted ) > 0 )
		AGameStatus::GetSingleton()->GetLevelData().AddLandmarksCompleted( 0, -TINT( m_uiNumLandmarksCompleted ) );

	m_uiNumLandmarksCompleted = 0;
}

// $deBlob: FUNCTION 005a0150
void AUberChallenge::Update( TFLOAT a_fDeltaTime )
{
	AChallenge::Update( a_fDeltaTime );

	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 005a0190
void AUberChallenge::LinkSubChallenges()
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	TASSERT( pData->m_iSubChallengeGUID != -1 );

	// The sub challenges have no marker of their own
	for ( AChallenge* pChallenge = pData->m_pSubChallenge; pChallenge != TNULL; pChallenge = pChallenge->GetData()->m_ppLinkedChallenges[ 0 ] )
	{
		TASSERT( pChallenge->GetData()->m_uiNumLinkedChallenges <= 1, "Sub challenges must be a daisy chain of single links!" );
		pChallenge->GetRevolutionary()->SetVisible( TFALSE );
		pChallenge->GetData()->m_uiFlags |= Data::Flags_SubChallenge;

		if ( pChallenge->GetData()->m_uiNumLinkedChallenges == 0 )
			break;
	}
}

// $deBlob: FUNCTION 005a0230
void AUberChallenge::OnSubChallengeCompleted( AChallenge* a_pSubChallenge )
{
	TASSERT( a_pSubChallenge == m_pCurrSubChallenge );

	if ( m_pCurrSubChallenge->GetData()->m_uiNumLinkedChallenges == 0 )
	{
		// The last one: the whole challenge is completed
		AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );
		return;
	}

	// The next one starts with the time left, its hint is shown if it has one
	m_pCurrSubChallenge = m_pCurrSubChallenge->GetData()->m_ppLinkedChallenges[ 0 ];
	m_pCurrSubChallenge->Start();
	m_pCurrSubChallenge->SetTimeLeft( m_fTimeLeft );

	Data* pSubData = m_pCurrSubChallenge->GetData();

	if ( pSubData->m_iStringID != -1 && pSubData->m_iSSCamera == -1 )
	{
		APropTriggerHelp::SetUnk9B720C( TTRUE );
		APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Scene, m_pCurrSubChallenge->GetData()->m_iStringID, -1, TTRUE, 4.0f, 0, TNULL, TTRUE );
	}
}

// $deBlob: FUNCTION 005a0300
void AUberChallenge::OnSubChallengeTimeOut( AChallenge* a_pSubChallenge )
{
	TASSERT( a_pSubChallenge == m_pCurrSubChallenge );
	AChallengeManager::GetSingleton()->FailCurrentChallenge();
}

// $deBlob: FUNCTION 005a0350
void AUberChallenge::OnPaintEvent( void* a_pBuilding )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnPaintEvent( a_pBuilding );
}

// $deBlob: FUNCTION 005a0370
void AUberChallenge::OnZJumpEvent( void* a_pSet )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnZJumpEvent( a_pSet );
}

// $deBlob: FUNCTION 005a0390
void AUberChallenge::OnCombatEvent( void* a_pEntityID )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnCombatEvent( a_pEntityID );
}

// $deBlob: FUNCTION 005a03b0
void AUberChallenge::OnCivilianEvent( void* a_pEntityID )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnCivilianEvent( a_pEntityID );
}

// $deBlob: FUNCTION 005a03d0
void AUberChallenge::OnCarEvent( void* a_pEntityID )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnCarEvent( a_pEntityID );
}

// $deBlob: FUNCTION 005a03f0
void AUberChallenge::OnLandmarkEvent( void* a_pLandmark )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnLandmarkEvent( a_pLandmark );
}

// $deBlob: FUNCTION 005a0410
void AUberChallenge::OnLandmarkEvent2( void* a_pLandmarkInstance )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnLandmarkEvent2( a_pLandmarkInstance );
}

// $deBlob: FUNCTION 005a0430
void AUberChallenge::OnPaintEvent2( void* a_pTree )
{
	if ( m_pCurrSubChallenge != TNULL )
		m_pCurrSubChallenge->OnPaintEvent2( a_pTree );
}
