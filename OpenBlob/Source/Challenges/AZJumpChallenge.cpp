#include "pch.h"
#include "AZJumpChallenge.h"
#include "AChallengeManager.h"
#include "Entities/AZJumpData.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0059c910
AZJumpChallenge::AZJumpChallenge()
    : AChallenge( Type_ZJump )
{

}

// $deBlob: FUNCTION 005a0470
AZJumpChallenge::~AZJumpChallenge()
{

}

// $deBlob: FUNCTION 005a0480
TBOOL AZJumpChallenge::Create( TypeData* a_pData )
{
	if ( !AChallenge::Create( a_pData ) )
		return TFALSE;

	AZJumpData* pZJumpData = AZJumpData::GetSingleton();

	for ( TUINT32 i = 0; i < a_pData->m_uiNumSets; i++ )
	{
		TASSERT( -1 != a_pData->m_aiZJumpSets[ i ].m_iWPSetID );

		// The index of the set in the TRB becomes the set itself
		a_pData->m_aiZJumpSets[ i ].m_pSet  = pZJumpData->GetSet( a_pData->m_aiZJumpSets[ i ].m_iWPSetID );
		a_pData->m_aiZJumpSets[ i ].m_bDone = TFALSE;
	}

	return TTRUE;
}

// A set was jumped through (a_pSet: the AZJumpData set), TNULL: the time ran out
// $deBlob: FUNCTION 005a0590
void AZJumpChallenge::OnZJumpEvent( void* a_pSet )
{
	if ( a_pSet != TNULL )
	{
		TypeData* pData = TSTATICCAST( TypeData, m_pData );

		for ( TUINT32 i = 0; i < pData->m_uiNumSets; i++ )
		{
			if ( pData->m_aiZJumpSets[ i ].m_pSet == a_pSet )
				pData->m_aiZJumpSets[ i ].m_bDone = TTRUE;
		}
	}
	else
	{
		AChallengeManager::GetSingleton()->OnChallengeTimeOut( this );
	}
}

// $deBlob: FUNCTION 005a05d0
void AZJumpChallenge::Update( TFLOAT a_fDeltaTime )
{
	AChallenge::Update( a_fDeltaTime );

	if ( ( m_uiFlags & Flags_Completed ) || ( m_uiFlags & Flags_Failed ) )
		return;

	// Completed once every set is done
	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	TBOOL     bDone = TTRUE;

	for ( TUINT32 i = 0; i < pData->m_uiNumSets; i++ )
	{
		if ( !pData->m_aiZJumpSets[ i ].m_bDone )
			bDone = TFALSE;
	}

	if ( bDone == TTRUE )
		AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );
}

// $deBlob: FUNCTION 005a0640
void AZJumpChallenge::Start()
{
	TSTATICCAST( ABlobChallengeStatusHud, AChallengeManager::GetSingleton()->GetHudElement() )->OnChallengeStart( this );
	AChallengeManager::GetSingleton()->SetCounter( -1, -1 );

	AChallenge::Start();
}

// $deBlob: FUNCTION 005a0670
void AZJumpChallenge::Reset( TBOOL a_bUnk )
{
	AChallenge::Reset( a_bUnk );

	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumSets; i++ )
		pData->m_aiZJumpSets[ i ].m_bDone = TFALSE;
}
