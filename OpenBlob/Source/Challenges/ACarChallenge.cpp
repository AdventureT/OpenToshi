#include "pch.h"
#include "ACarChallenge.h"
#include "AChallengeManager.h"
#include "Entities/ACar/ACar.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0059c590
ACarChallenge::ACarChallenge()
    : AChallenge( Type_Car )
{

}

// $deBlob: FUNCTION 00597ef0
TBOOL ACarChallenge::Create( TypeData* a_pData )
{
	if ( !AChallenge::Create( a_pData ) )
		return TFALSE;

	for ( TUINT32 i = 0; i < a_pData->m_uiNumEntries; i++ )
	{
		TINT iID = a_pData->m_pEntries[ i ].m_iID;
		TASSERT( iID != -1 );

		ABaseEntity* pEntity = ABaseEntity::FindIDEntity( iID );
		TASSERT( pEntity->IsA( &TGetClass( ACar ) ), "Car Challenge was linked to an entity that was not a Car!" );

		a_pData->m_pEntries[ i ].m_pEntity = pEntity;
		a_pData->m_pEntries[ i ].m_iDone   = 0;
	}

	return TTRUE;
}

// A car was painted (a_pEntityID: its entity ID)
// $deBlob: FUNCTION 00598010
void ACarChallenge::OnCarEvent( void* a_pEntityID )
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( pData->m_pEntries[ i ].m_iID == TINT( TUINTPTR( a_pEntityID ) ) )
			pData->m_pEntries[ i ].m_iDone = 1;
	}

	TINT iPainted = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( pData->m_pEntries[ i ].m_iDone != 0 )
			iPainted++;
	}

	AChallengeManager::GetSingleton()->SetCounter( iPainted, pData->m_uiNumEntries );
}

// $deBlob: FUNCTION 00598070
void ACarChallenge::Update( TFLOAT a_fDeltaTime )
{
	AChallenge::Update( a_fDeltaTime );

	if ( ( m_uiFlags & Flags_Completed ) || ( m_uiFlags & Flags_Failed ) )
		return;

	// Completed once every car is painted
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( pData->m_pEntries[ i ].m_iDone == 0 )
			return;
	}

	AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );
}

// $deBlob: FUNCTION 005980d0
void ACarChallenge::Reset( TBOOL a_bUnk )
{
	AChallenge::Reset( a_bUnk );

	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
		pData->m_pEntries[ i ].m_iDone = 0;
}

// $deBlob: FUNCTION 00598110
void ACarChallenge::Start()
{
	TypeData* pData    = TSTATICCAST( TypeData, m_pData );
	TINT      iPainted = 0;

	// The cars already painted count
	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( TSTATICCAST( AProp, pData->m_pEntries[ i ].m_pEntity )->IsPainted() )
		{
			iPainted++;
			pData->m_pEntries[ i ].m_iDone = 1;
		}
	}

	AChallengeManager* pManager = AChallengeManager::GetSingleton();
	TSTATICCAST( ABlobChallengeStatusHud, pManager->GetHudElement() )->OnChallengeStart( this );
	pManager->SetCounter( iPainted, pData->m_uiNumEntries );

	AChallenge::Start();
}

// The share of the cars painted (percent); NOTE: divides by zero without entries
// $deBlob: FUNCTION 00598190
TINT ACarChallenge::GetScore()
{
	TypeData* pData     = TSTATICCAST( TypeData, m_pData );
	TUINT32   uiPainted = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( TSTATICCAST( AProp, pData->m_pEntries[ i ].m_pEntity )->IsPainted() )
			uiPainted++;
	}

	return TINT( ( TFLOAT( uiPainted ) / TFLOAT( pData->m_uiNumEntries ) ) * 100.0f );
}

// Are all the cars painted?
// $deBlob: FUNCTION 00598220
TBOOL ACarChallenge::IsAlreadyCompleted()
{
	TypeData* pData     = TSTATICCAST( TypeData, m_pData );
	TUINT32   uiPainted = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( TSTATICCAST( AProp, pData->m_pEntries[ i ].m_pEntity )->IsPainted() )
			uiPainted++;
	}

	return uiPainted == pData->m_uiNumEntries;
}
