#include "pch.h"
#include "ACivilianChallenge.h"
#include "AChallengeManager.h"
#include "Entities/ACiv.h"
#include "GameController/AGameStatus.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0059c650
ACivilianChallenge::ACivilianChallenge()
    : AChallenge( Type_Civilian )
{

}

// $deBlob: FUNCTION 0059d180
TBOOL ACivilianChallenge::Create( TypeData* a_pData )
{
	if ( !AChallenge::Create( a_pData ) )
		return TFALSE;

	for ( TUINT32 i = 0; i < a_pData->m_uiNumEntries; i++ )
	{
		TINT iID = a_pData->m_pEntries[ i ].m_iID;
		TASSERT( iID != -1 );

		ABaseEntity* pEntity = ABaseEntity::FindIDEntity( iID );
		TASSERT( pEntity->IsA( &TGetClass( ACiv ) ), "Civilian Challenge was linked to an entity that was not a Civ!" );

		a_pData->m_pEntries[ i ].m_pEntity = pEntity;
		a_pData->m_pEntries[ i ].m_iDone   = 0;
	}

	return TTRUE;
}

// A civilian was freed (a_pEntityID: its entity ID)
// $deBlob: FUNCTION 0059d2a0
void ACivilianChallenge::OnCivilianEvent( void* a_pEntityID )
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( pData->m_pEntries[ i ].m_iID == TINT( TUINTPTR( a_pEntityID ) ) )
			pData->m_pEntries[ i ].m_iDone = 1;
	}

	TINT iFreed = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( pData->m_pEntries[ i ].m_iDone != 0 )
			iFreed++;
	}

	AChallengeManager::GetSingleton()->SetCounter( iFreed, pData->m_uiNumEntries );
}

// $deBlob: FUNCTION 0059d300
void ACivilianChallenge::Update( TFLOAT a_fDeltaTime )
{
	AChallenge::Update( a_fDeltaTime );

	if ( ( m_uiFlags & Flags_Completed ) || ( m_uiFlags & Flags_Failed ) )
		return;

	// Completed once every civilian is freed
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( pData->m_pEntries[ i ].m_iDone == 0 )
			return;
	}

	AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );
}

// $deBlob: FUNCTION 0059d360
void ACivilianChallenge::Reset( TBOOL a_bUnk )
{
	TypeData* pData  = TSTATICCAST( TypeData, m_pData );
	TINT      iFreed = 0;

	if ( pData->m_uiNumEntries != 0 )
	{
		for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
		{
			if ( pData->m_pEntries[ i ].m_iDone != 0 )
				iFreed++;

			pData->m_pEntries[ i ].m_iDone = 0;
		}

		// The civilians freed during the challenge don't count
		if ( iFreed > 0 )
			AGameStatus::GetSingleton()->GetLevelData().AddCivsFreed( 0, -iFreed );
	}

	AChallenge::Reset( a_bUnk );
}

// $deBlob: FUNCTION 0059d3c0
void ACivilianChallenge::Start()
{
	TypeData* pData  = TSTATICCAST( TypeData, m_pData );
	TINT      iFreed = 0;

	// The civilians already freed count
	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( TSTATICCAST( ACiv, pData->m_pEntries[ i ].m_pEntity )->IsTransformed() )
		{
			iFreed++;
			pData->m_pEntries[ i ].m_iDone = 1;
		}
	}

	AChallengeManager* pManager = AChallengeManager::GetSingleton();
	TSTATICCAST( ABlobChallengeStatusHud, pManager->GetHudElement() )->OnChallengeStart( this );
	pManager->SetCounter( iFreed, pData->m_uiNumEntries );

	AChallenge::Start();
}

// The share of the civilians freed (percent); NOTE: divides by zero without entries
// $deBlob: FUNCTION 0059d440
TINT ACivilianChallenge::GetScore()
{
	TypeData* pData  = TSTATICCAST( TypeData, m_pData );
	TUINT32   uiFreed = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( TSTATICCAST( ACiv, pData->m_pEntries[ i ].m_pEntity )->IsTransformed() )
			uiFreed++;
	}

	return TINT( ( TFLOAT( uiFreed ) / TFLOAT( pData->m_uiNumEntries ) ) * 100.0f );
}

// Are all the civilians freed?
// $deBlob: FUNCTION 0059d4c0
TBOOL ACivilianChallenge::IsAlreadyCompleted()
{
	TypeData* pData   = TSTATICCAST( TypeData, m_pData );
	TUINT32   uiFreed = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( TSTATICCAST( ACiv, pData->m_pEntries[ i ].m_pEntity )->IsTransformed() )
			uiFreed++;
	}

	return uiFreed == pData->m_uiNumEntries;
}
