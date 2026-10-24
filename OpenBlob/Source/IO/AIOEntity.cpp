#include "pch.h"
#include "AIOEntity.h"
#include "IO/AIOMeshComponent.h"
#include "IO/AIOCollisionComponent.h"
#include "IO/AIOEnemyComponent.h"
#include "IO/AIOKeyframeComponent.h"
#include "IO/AIOMovieComponent.h"
#include "IO/AIOPopupComponent.h"
#include "Sound/ASoundComponent.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Animation/AModel2.h"
#include "Entities/AFXBreakpointComponent.h"
#include "IO/AIOTemplateManager.h"
#include "IO/IOFadeUpdater.h"
#include "IO/IOMoveUpdater.h"
#include "IO/IORotateUpdater.h"
#include "IO/IOScaleUpdater.h"
#include "IO/IOShakeUpdater.h"
#include "Entities/AAttackMessage.h"
#include "Entities/Projectiles/AProjectile.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "MemoryCard/AAutosaveManager.h"
#include "ARandom.h"
#include "Entities/APickup.h"
#include "Entities/APickupData.h"

#include <Render/TModel.h>
#include <Platform/DX11/TRender_DX11.h>

#include <Toshi/TStringManager.h>
#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AIOEntity::t_IOCallback AIOEntity::sm_aIOCallbacks[ NUM_IOCALLBACKS ];

// $deBlob: FUNCTION 007832b0
// $deBlob: FUNCTION 007832c0
// $deBlob: FUNCTION 00783330
// $deBlob: FUNCTION 00540ba0
// $deBlob: FUNCTION 00859d60
TDEFINE_CLASS_INIT( AIOEntity );

// $deBlob: FUNCTION 007833c0
AIOEntity::AIOEntity()
{
	m_oLocalMatrix.Identity();
	m_pIOData         = TNULL;
	m_pC4Items        = TNULL;
	m_uiC4Count       = 0;
	m_pCCList         = TNULL;
	m_pD0Component    = TNULL;
	m_iD4Count        = 0;
	m_pD8Items        = TNULL; // NOTE: retail leaves it unset, it's only read while m_iD4Count > 0
	m_iDCCount        = 0;
	m_pE0Items        = TNULL; // NOTE: retail leaves it unset, it's only read while m_iDCCount > 0
	m_uiFlags         = 0;
	m_uiColour        = 0xFFFFFF00;
	m_fHeartBeat      = 0.0f;
	m_fHeartBeatTimer = 0.0f;
	m_iHitPoints      = 0;
	m_iCounter        = 0;
	m_pF8List         = TNULL;
	m_pFCList         = TNULL;
	m_p100List        = TNULL;
	m_pLastTarget     = TNULL;

	SetBBox( Toshi::TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
	m_uiFlags |= 0x100;
}

// $deBlob: FUNCTION 007834f0
// $deBlob: FUNCTION 00786a20
AIOEntity::~AIOEntity()
{
	// 0x100: the updaters, the next pointer is read before each delete
	while ( m_p100List != TNULL )
	{
		IIOMessageUpdater* pUpdater = m_p100List;
		IIOMessageUpdater* pNext    = pUpdater->GetNext();

		if ( pUpdater != TNULL )
			delete pUpdater;

		m_p100List = pNext;
	}

	// 0xF8: the FX compositions
	while ( m_pF8List != TNULL )
	{
		FXComposition* pFX   = m_pF8List;
		FXComposition* pNext = pFX->m_pNext;

		if ( pFX != TNULL )
			delete pFX;

		m_pF8List = pNext;
	}

	// 0xFC: the components, the next pointer is read before each delete
	while ( m_pFCList != TNULL )
	{
		AIOEntityComponent* pComponent = m_pFCList;
		m_pFCList                      = pComponent->GetNext();
		delete pComponent;
	}

	// 0xC4: the target lists, their targets are freed before the array
	if ( m_pC4Items != TNULL )
	{
		for ( TINT i = 0; i < m_uiC4Count; i++ )
			delete[] m_pC4Items[ i ].pTargets;

		delete[] m_pC4Items;
		m_pC4Items = TNULL;
	}

	// 0xCC: a list of nodes with the next pointer at +8
	while ( m_pCCList != TNULL )
	{
		void* pNode = m_pCCList;
		m_pCCList   = *reinterpret_cast<void**>( static_cast<TUINT8*>( pNode ) + 8 );
		TFree( pNode );
	}

	if ( m_iDCCount > 0 && m_pE0Items != TNULL )
		delete[] m_pE0Items;

	if ( m_iD4Count > 0 && m_pD8Items != TNULL )
		delete[] m_pD8Items;

	if ( m_pD0Component != TNULL )
		delete m_pD0Component;
}

// $deBlob: FUNCTION 00783820
void AIOEntity::SetIOEntityData( const IOEntityData& a_rData, TTRB* a_pTRB )
{
	SetID( a_rData.iID );
	SetName( a_rData.szName );

	if ( a_rData.szTemplate != TNULL )
		m_pIOData = AIOTemplateManager::GetSingleton()->GetIOTemplateReference( a_rData.szTemplate );

	// The low byte of m_iCollisionGroupAndType: the projectiles' collision byte (AProjectile::UpdateLifeTime)
	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF00 ) | 0x20 );

	SetupIOEntityComponents( a_rData, a_pTRB );

	TMatrix44   matRotation;
	TQuaternion quatRotation = *TREINTERPRETCAST( const TQuaternion*, a_rData.pRotation );
	matRotation.SetFromQuaternion( quatRotation );
	SetMatrix( matRotation );
	SetPosition( *a_rData.pPosition );

	m_oLocalMatrix = m_oMatrix;

	SetupTargets( a_rData );

	if ( GetModelComponent() != TNULL )
	{
		AModel2* pModel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel();

		TASSERT( pModel->GetFileName().IsIndexValid( 0 ) );
		m_pD0Component = AFXBreakpointComponent::Create( pModel->GetFileName(), TMemory_GetGlobalHeap(), this, TNULL );
	}

	SetVisible( TTRUE );
	SetActive( TTRUE );
}

// $deBlob: FUNCTION 007836d0
void AIOEntity::Create()
{
	if ( m_pIOData )
	{
		SetupDelaysAndCooldowns();
		SetupAttackableInteractFlags();
		SetupFXCompositions();
		SetupAnimIDs();
		SetupProjectilePools();
	}

	ABaseEntity::Create();
}

// Copies the data's target lists, PostCreate looks their entities up
// $deBlob: FUNCTION 00785930
void AIOEntity::SetupTargets( const IOEntityData& a_rData )
{
	m_uiC4Count = a_rData.uiNumTargets;

	if ( m_uiC4Count == 0 )
		return;

	m_pC4Items = new TargetList[ m_uiC4Count ];

	for ( TINT i = 0; i < m_uiC4Count; i++ )
	{
		m_pC4Items[ i ].iUnk00      = a_rData.pTargets[ i ].iUnk00;
		m_pC4Items[ i ].iUnk04      = a_rData.pTargets[ i ].iUnk04;
		m_pC4Items[ i ].iNumTargets = a_rData.pTargets[ i ].iNumIDs;

		if ( m_pC4Items[ i ].iNumTargets != 0 )
		{
			m_pC4Items[ i ].pTargets = new Target[ m_pC4Items[ i ].iNumTargets ];

			for ( TINT j = 0; j < m_pC4Items[ i ].iNumTargets; j++ )
				m_pC4Items[ i ].pTargets[ j ].iID = a_rData.pTargets[ i ].pIDs[ j ];
		}
		else
		{
			m_pC4Items[ i ].pTargets = TNULL;
		}
	}
}

// The flags of the IO data's input types: 0x80 when it has input 0x33, attackable (0x40, also on the entity's m_Flags)
// when it has one of the inputs 8, 9, 10 or 0x2E
// $deBlob: FUNCTION 00785a90
void AIOEntity::SetupAttackableInteractFlags()
{
	// NOTE: the IO data isn't typed yet: its input count at +0x4, its 0x1C byte inputs at +0x8 (the type first)
	TUINT8* pIOData    = TSTATICCAST( TUINT8, m_pIOData );
	TINT    iNumInputs = *TREINTERPRETCAST( TINT*, pIOData + 4 );
	TUINT8* pInputs    = *TREINTERPRETCAST( TUINT8**, pIOData + 8 );

	for ( TINT i = 0; i < iNumInputs; i++ )
	{
		TINT iType = *TREINTERPRETCAST( TINT*, pInputs + i * 0x1C );

		if ( iType == 0x33 )
		{
			m_uiFlags |= 0x80;
		}
		else if ( iType == 8 || iType == 9 || iType == 10 || iType == 0x2E )
		{
			m_uiFlags |= 0x40;
			m_Flags |= 0x40;
		}
	}
}

// One ActionDelay per action with a delay, one InputCooldown per input with a cooldown
// NOTE: the IO data isn't typed yet: the inputs (0x1C bytes) have their cooldown at +0xC, their action count at +0x4 and
// their 0xC byte actions at +0x8 (the type first, the delay at +0x4, the params at +0x8)
// $deBlob: FUNCTION 00785af0
void AIOEntity::SetupDelaysAndCooldowns()
{
	for ( TINT i = 0; i < *TREINTERPRETCAST( TINT*, TSTATICCAST( TUINT8, m_pIOData ) + 4 ); i++ )
	{
		TUINT8* pInput = *TREINTERPRETCAST( TUINT8**, TSTATICCAST( TUINT8, m_pIOData ) + 8 ) + i * 0x1C;

		if ( *TREINTERPRETCAST( TFLOAT*, pInput + 0xC ) > 0.0f )
			m_iDCCount++;

		for ( TINT j = 0; j < *TREINTERPRETCAST( TINT*, pInput + 4 ); j++ )
		{
			TUINT8* pAction = *TREINTERPRETCAST( TUINT8**, pInput + 8 ) + j * 0xC;

			if ( *TREINTERPRETCAST( TFLOAT*, pAction + 4 ) > 0.0f )
				m_iD4Count++;
		}
	}

	if ( m_iD4Count > 0 )
		m_pD8Items = new ActionDelay[ m_iD4Count ];

	if ( m_iDCCount > 0 )
		m_pE0Items = new InputCooldown[ m_iDCCount ];
}

// Creates the projectile pools of the fire actions (0x41, the projectile type is the first param)
// $deBlob: FUNCTION 00785c60
void AIOEntity::SetupProjectilePools()
{
	for ( TINT i = 0; i < *TREINTERPRETCAST( TINT*, TSTATICCAST( TUINT8, m_pIOData ) + 4 ); i++ )
	{
		TUINT8* pInput = *TREINTERPRETCAST( TUINT8**, TSTATICCAST( TUINT8, m_pIOData ) + 8 ) + i * 0x1C;

		for ( TINT j = 0; j < *TREINTERPRETCAST( TINT*, pInput + 4 ); j++ )
		{
			TUINT8* pAction = *TREINTERPRETCAST( TUINT8**, pInput + 8 ) + j * 0xC;

			if ( *TREINTERPRETCAST( TINT*, pAction ) == 0x41 )
			{
				TINT* pParams = *TREINTERPRETCAST( TINT**, pAction + 8 );
				AProjectileManager::GetSingleton()->InitPool( AProjectileTypes::PROJECTILETYPES( pParams[ 0 ] ), 50 );
			}
		}
	}
}

// One FXComposition per FX id of the FX actions (0x1C; params: the name at +0x8, the id at +0xC, a flag at +0x10)
// $deBlob: FUNCTION 00785ce0
void AIOEntity::SetupFXCompositions()
{
	for ( TINT i = 0; i < *TREINTERPRETCAST( TINT*, TSTATICCAST( TUINT8, m_pIOData ) + 4 ); i++ )
	{
		TUINT8* pInput = *TREINTERPRETCAST( TUINT8**, TSTATICCAST( TUINT8, m_pIOData ) + 8 ) + i * 0x1C;

		for ( TINT j = 0; j < *TREINTERPRETCAST( TINT*, pInput + 4 ); j++ )
		{
			TUINT8* pAction = *TREINTERPRETCAST( TUINT8**, pInput + 8 ) + j * 0xC;

			if ( *TREINTERPRETCAST( TINT*, pAction ) != 0x1C )
				continue;

			TUINT8*      pParams = *TREINTERPRETCAST( TUINT8**, pAction + 8 );
			TINT         iID     = *TREINTERPRETCAST( TINT*, pParams + 0xC );
			const TCHAR* szName  = *TREINTERPRETCAST( const TCHAR**, pParams + 8 );

			if ( m_pF8List == TNULL )
			{
				m_pF8List                   = new FXComposition();
				m_pF8List->m_iID            = iID;
				m_pF8List->m_bFollowsMatrix = *TREINTERPRETCAST( TINT*, pParams + 0x10 ) != 0;
				m_pF8List->m_szName         = szName;
				m_pF8List->m_oInstance.Initialize( szName );
			}
			else
			{
				FXComposition* pFX = m_pF8List;
				FXComposition* pNext;

				do
				{
					if ( pFX->m_iID == iID )
					{
						// NOTE: the result is unused, an id used again with another name keeps the first one
						TStringManager::String8CompareNoCase( pFX->m_szName, szName );
						break;
					}

					pNext = pFX->m_pNext;

					if ( pNext == TNULL )
					{
						FXComposition* pNewFX    = new FXComposition();
						pNewFX->m_iID            = iID;
						pNewFX->m_bFollowsMatrix = *TREINTERPRETCAST( TINT*, pParams + 0x10 ) != 0;
						pNewFX->m_szName         = szName;
						pNewFX->m_oInstance.Initialize( szName );
						pFX->m_pNext = pNewFX;
						break;
					}

					pFX = pNext;
				} while ( pNext != TNULL );
			}
		}
	}
}

// The anim IDs of the play anim actions (0x1B, the anim name is the first param), when the entity has a model
// $deBlob: FUNCTION 007862f0
void AIOEntity::SetupAnimIDs()
{
	if ( GetModelComponent() == TNULL )
		return;

	AToshiAnimInterface2* pAnimInterface = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();

	for ( TINT i = 0; i < *TREINTERPRETCAST( TINT*, TSTATICCAST( TUINT8, m_pIOData ) + 4 ); i++ )
	{
		TUINT8* pInput = *TREINTERPRETCAST( TUINT8**, TSTATICCAST( TUINT8, m_pIOData ) + 8 ) + i * 0x1C;

		for ( TINT j = 0; j < *TREINTERPRETCAST( TINT*, pInput + 4 ); j++ )
		{
			TUINT8* pAction = *TREINTERPRETCAST( TUINT8**, pInput + 8 ) + j * 0xC;

			if ( *TREINTERPRETCAST( TINT*, pAction ) == 0x1B )
			{
				const TCHAR** pParams = *TREINTERPRETCAST( const TCHAR***, pAction + 8 );

				// NOTE: the node isn't typed yet: the anim ID (a TINT16 in a TINT), the action, the next
				TINT* pNode = TSTATICCAST( TINT, TMalloc( 0xC ) );
				pNode[ 0 ]  = 0;
				pNode[ 1 ]  = 0;
				pNode[ 2 ]  = 0;
				pNode[ 1 ]  = TINT( TREINTERPRETCAST( TUINTPTR, pAction ) );
				pNode[ 0 ]  = TINT16( pAnimInterface->GetAnimID( pParams[ 0 ] ) );
				pNode[ 2 ]  = TINT( TREINTERPRETCAST( TUINTPTR, m_pCCList ) );
				m_pCCList   = pNode;
			}
		}
	}
}

// $deBlob: FUNCTION 007863f0
void AIOEntity::AddMessageUpdater( IIOMessageUpdater* a_pUpdater )
{
	a_pUpdater->SetNext( m_p100List );
	m_p100List = a_pUpdater;
}

// Creates the components of the data (their type is the first int of each ComponentData) and gives each its data
// $deBlob: FUNCTION 00786410
void AIOEntity::SetupIOEntityComponents( const IOEntityData& a_rData, TTRB* a_pTRB )
{

	for ( TINT i = 0; i < a_rData.iNumComponents; i++ )
	{
		ComponentData*      pData      = &a_rData.pComponents[ i ];
		AIOEntityComponent* pComponent = TNULL;

		switch ( pData->iType )
		{
			case ASoundComponent::TYPE:
				pComponent = new ASoundComponent( this );
				break;
			case AIOEnemyComponent::TYPE:
				pComponent = new AIOEnemyComponent( this );
				break;
			case AIOPopupComponent::TYPE:
				pComponent = new AIOPopupComponent( this );
				break;
			case AIOMovieComponent::TYPE:
				pComponent = new AIOMovieComponent( this );
				break;
			case AIOMeshComponent::TYPE:
				pComponent = new AIOMeshComponent( this );
				break;
			case AIOCollisionComponent::TYPE:
				pComponent = new AIOCollisionComponent( this );
				break;
			case AIOKeyframeComponent::KEYFRAME_COMPONENT:
				pComponent = new AIOKeyframeComponent( this );
				break;
		}

		// NOTE: retail doesn't check the component: an unknown type (or a failed allocation) crashes here
		pComponent->SetNext( m_pFCList );
		m_pFCList = pComponent;
		pComponent->SetIOEntityData( pData, a_pTRB );
	}
}

// Creates the updaters of the actions that run over time (rotate, move, fade, scale and shake; a shake also shakes the
// entities of the action's target list)
// $deBlob: FUNCTION 00785f10
void AIOEntity::SetupUpdateOverTimeMessages()
{
	TUINT8* pIOData = TSTATICCAST( TUINT8, m_pIOData );

	for ( TINT i = 0; i < *TREINTERPRETCAST( TINT*, pIOData + 4 ); i++ )
	{
		TUINT8* pInput = *TREINTERPRETCAST( TUINT8**, pIOData + 8 ) + i * 0x1C;

		for ( TINT j = 0; j < *TREINTERPRETCAST( TINT*, pInput + 4 ); j++ )
		{
			TUINT8*     pAction  = *TREINTERPRETCAST( TUINT8**, pInput + 8 ) + j * 0xC;
			TargetList* pTargets = FindTargetList( pInput, pAction );
			TFLOAT      fParam   = **TREINTERPRETCAST( TFLOAT**, pAction + 8 );

			IIOMessageUpdater* pUpdater = TNULL;

			switch ( *TREINTERPRETCAST( TINT*, pAction ) )
			{
				case 0x27:
				{
					IOShakeUpdater* pShake = new IOShakeUpdater();
					pShake->SetOwner( this );
					pShake->SetAction( pAction );
					pShake->SetInput( pInput );
					pShake->SetNext( m_p100List );
					m_p100List = pShake;

					if ( pTargets != TNULL )
					{
						for ( TINT k = 0; k < pTargets->iNumTargets; k++ )
						{
							if ( pTargets->pTargets[ k ].pEntity != TNULL )
							{
								IOShakeUpdater* pTargetShake = new IOShakeUpdater();
								pTargetShake->SetOwner( TSTATICCAST( AIOEntity, pTargets->pTargets[ k ].pEntity ) );
								pTargetShake->SetAction( pAction );

								ABaseEntity* pTarget = pTargets->pTargets[ k ].pEntity;
								TASSERT( ( pTarget == TNULL ) || pTarget->IsA( &TGetClass( AIOEntity ) ) );
								AIOEntity* pTargetIO = TSTATICCAST( AIOEntity, pTarget );

								pTargetShake->SetNext( pTargetIO->m_p100List );
								pTargetIO->m_p100List = pTargetShake;
							}
						}
					}

					break;
				}
				case 0x3B:
					if ( 0.0f < fParam || fParam == -1.0f )
						pUpdater = new IORotateUpdater();

					break;
				case 0x3C:
					if ( 0.0f < fParam || fParam == -1.0f )
					{
						IOMoveUpdater* pMove = new IOMoveUpdater();
						pMove->SetOwner( this );
						pMove->SetAction( pAction );
						pMove->SetInput( pInput );
						pMove->SetNext( m_p100List );
						m_p100List = pMove;

						// To the first target
						if ( pTargets != TNULL && pTargets->iNumTargets != 0 )
							pMove->SetTarget( pTargets->pTargets[ 0 ].pEntity );
					}

					break;
				case 0x3E:
					if ( fParam != 0.0f )
						pUpdater = new IOFadeUpdater();

					break;
				case 0x3F:
					if ( fParam != 0.0f )
						pUpdater = new IOScaleUpdater();

					break;
			}

			if ( pUpdater != TNULL )
			{
				pUpdater->SetOwner( this );
				pUpdater->SetAction( pAction );
				pUpdater->SetInput( pInput );
				pUpdater->SetNext( m_p100List );
				m_p100List = pUpdater;
			}
		}
	}
}

// $deBlob: FUNCTION 00783710
void AIOEntity::PostCreate()
{
	// The entities of the target lists
	for ( TINT i = 0; i < m_uiC4Count; i++ )
	{
		for ( TINT j = 0; j < m_pC4Items[ i ].iNumTargets; j++ )
			m_pC4Items[ i ].pTargets[ j ].pEntity = ABaseEntity::FindIDEntity( m_pC4Items[ i ].pTargets[ j ].iID );
	}

	if ( m_pIOData != TNULL )
		SetupUpdateOverTimeMessages();

	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
		pComponent->Unk04();

	TINT aMessage[ 3 ] = { 0, 0, 0 };
	HandleInputMessage( aMessage, this );
}

// $deBlob: FUNCTION 00783960
void AIOEntity::Tick( TFLOAT a_fDeltaTime )
{
	if ( ( m_uiFlags & 0x10 ) != 0 )
	{
		// Stays ticking while a timer runs, an updater or a component is active or an FX composition plays
		TBOOL bTicking = TFALSE;

		for ( TINT i = 0; i < m_iDCCount; i++ )
		{
			if ( m_pE0Items[ i ].m_fTimer > 0.0f )
			{
				m_pE0Items[ i ].m_fTimer -= a_fDeltaTime;

				if ( m_pE0Items[ i ].m_fTimer <= 0.0f )
					m_pE0Items[ i ].m_pInput = TNULL;

				bTicking = TTRUE;
			}
		}

		for ( TINT i = 0; i < m_iD4Count; i++ )
		{
			ActionDelay* pDelay = &m_pD8Items[ i ];

			if ( pDelay->m_fTimer > 0.0f )
			{
				pDelay->m_fTimer -= a_fDeltaTime;

				if ( pDelay->m_fTimer <= 0.0f )
				{
					// The delayed action: its type is its first int
					TINT*        pAction    = TSTATICCAST( TINT, pDelay->m_pAction );
					t_IOCallback fnCallback = sm_aIOCallbacks[ pAction[ 0 ] ];

					if ( fnCallback != TNULL )
					{
						fnCallback( pAction, pDelay->m_pInput, this, pDelay->m_pOther );
					}
					else
					{
						for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
						{
							if ( pComponent->ExecuteActionInvoker( pAction, this, TSTATICCAST( ABaseEntity, pDelay->m_pOther ) ) == TTRUE )
								break;
						}
					}
				}

				bTicking = TTRUE;
			}
		}

		for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
		{
			if ( pUpdater->IsActive() )
			{
				pUpdater->Update( a_fDeltaTime );
				bTicking = TTRUE;
			}
		}

		if ( m_fHeartBeat > 0.0f )
		{
			m_fHeartBeatTimer += a_fDeltaTime;

			if ( m_fHeartBeatTimer >= m_fHeartBeat )
			{
				TINT aMessage[ 3 ] = { 0x32, 0, 0 };
				HandleInputMessage( aMessage, this );
				m_fHeartBeatTimer = 0.0f;
			}

			bTicking = TTRUE;
		}

		for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
		{
			if ( ( pComponent->GetFlags() & 1 ) != 0 )
			{
				pComponent->Tick( a_fDeltaTime );
				bTicking = TTRUE;
			}
		}

		for ( FXComposition* pFX = m_pF8List; pFX != TNULL; pFX = pFX->m_pNext )
		{
			if ( pFX->m_bFollowsMatrix && pFX->m_pFollowedEntity != TNULL )
				pFX->m_oInstance.SetTransform( *TREINTERPRETCAST( const TMatrix44*, TSTATICCAST( TUINT8, pFX->m_pFollowedEntity ) + 0x10 ) );

			pFX->m_oInstance.Update( a_fDeltaTime );

			if ( !pFX->m_oInstance.IsFinished() )
				bTicking = TTRUE;
		}

		if ( !bTicking )
			SetTicking( TFALSE );

		if ( m_pD0Component != TNULL )
			m_pD0Component->Tick( a_fDeltaTime );

		m_uiFlags &= ~0x100;
	}

	if ( ( m_uiFlags & 0x200 ) != 0 && ( m_uiFlags & 0x400 ) == 0 )
	{
		TINT aMessage[ 3 ] = { 7, 0, 0 };
		HandleInputMessage( aMessage, this );
		m_uiFlags &= ~0x200;
	}

	m_uiFlags &= ~0x400;
	ABaseEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00783df0
AIOEntityComponent* AIOEntity::FindComponent( TINT a_iType )
{
	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == a_iType )
			return pComponent;
	}

	return TNULL;
}

// Places the model of the first component of type 4 at the entity's matrix while the matrix needs an update
// $deBlob: FUNCTION 00783e20
void AIOEntity::PreRenderUpdate()
{
	if ( ( m_Flags & FLAG_UpdateMatrix ) == 0 )
		return;

	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOMeshComponent::TYPE )
		{
			TSTATICCAST( AIOMeshComponent, pComponent )->UpdateWorldTransform( m_oMatrix );
			return;
		}
	}
}

// $deBlob: FUNCTION 00783e50
void AIOEntity::OnLODChanging( TINT a_iLOD )
{
	// The first component of type 4 gets the entity's visibility (and, when visible, its matrix) on high LOD, hidden
	// otherwise. Then the entity sends the input message 0x10 (high LOD) or 0x11 (other LODs) to itself.
	if ( ( a_iLOD & FLAG_HighLOD ) != 0 )
	{
		for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
		{
			if ( pComponent->GetType() == AIOMeshComponent::TYPE )
			{
				TBOOL bVisible = ( m_Flags & FLAG_Visible ) != 0;
				TSTATICCAST( AIOMeshComponent, pComponent )->SetVisible( bVisible );

				if ( bVisible )
					TSTATICCAST( AIOMeshComponent, pComponent )->UpdateWorldTransform( m_oMatrix );

				break;
			}
		}

		TINT aMessage[ 3 ] = { 0x10, 0, 0 };
		HandleInputMessage( aMessage, this );
		return;
	}

	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOMeshComponent::TYPE )
		{
			TSTATICCAST( AIOMeshComponent, pComponent )->SetVisible( TFALSE );
			break;
		}
	}

	TINT aMessage[ 3 ] = { 0x11, 0, 0 };
	HandleInputMessage( aMessage, this );
}

// Shows or hides the entity, its first component of type 4 (placed at the entity's matrix when shown) and its FX
// breakpoints. Without such a component, the IO flag 0x100 lets AIOMeshComponent's tick follow the entity's visibility.
// $deBlob: FUNCTION 00784130
void AIOEntity::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible )
		m_Flags |= FLAG_Visible;
	else
		m_Flags &= ~FLAG_Visible;

	AIOEntityComponent* pComponent = m_pFCList;

	while ( pComponent != TNULL && pComponent->GetType() != AIOMeshComponent::TYPE )
		pComponent = pComponent->GetNext();

	if ( pComponent != TNULL )
	{
		TSTATICCAST( AIOMeshComponent, pComponent )->SetVisible( a_bVisible );

		if ( a_bVisible )
			TSTATICCAST( AIOMeshComponent, pComponent )->UpdateWorldTransform( m_oMatrix );
	}
	else
	{
		m_uiFlags |= 0x100;
	}

	if ( m_pD0Component != TNULL )
		m_pD0Component->SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 00784a10
AIOEntity::TargetList* AIOEntity::FindTargetList( void* a_pInput, void* a_pAction )
{
	TUINT8* pIOData = TSTATICCAST( TUINT8, m_pIOData );

	if ( pIOData == TNULL || m_pC4Items == TNULL )
		return TNULL;

	// The index of the input
	TINT iNumInputs = *TREINTERPRETCAST( TINT*, pIOData + 4 );

	if ( iNumInputs < 1 )
		return TNULL;

	TINT    iInput = 0;
	TUINT8* pInput = *TREINTERPRETCAST( TUINT8**, pIOData + 8 );

	while ( pInput != a_pInput )
	{
		iInput++;
		pInput += 0x1C;

		if ( iNumInputs <= iInput )
			return TNULL;
	}

	// The index of the action
	TINT iAction = -1;

	for ( TINT i = 0; i < *TREINTERPRETCAST( TINT*, pInput + 4 ); i++ )
	{
		if ( *TREINTERPRETCAST( TUINT8**, pInput + 8 ) + i * 0xC == a_pAction )
		{
			iAction = i;
			break;
		}
	}

	if ( iInput != -1 && iAction != -1 )
	{
		for ( TINT i = 0; i < m_uiC4Count; i++ )
		{
			if ( m_pC4Items[ i ].iUnk00 == iInput && m_pC4Items[ i ].iUnk04 == iAction )
				return &m_pC4Items[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00784bd0
void AIOEntity::HandleInputMessage( TINT* a_pMessage, ABaseEntity* a_pOther )
{
	if ( ( m_uiFlags & 0x10 ) != 0 )
	{
		if ( m_pIOData == TNULL )
		{
			for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
				pComponent->ExecuteActionInvoker( a_pMessage, this, a_pOther );

			return;
		}

		// The IO data's inputs with the message's id (the count is read again after each input)
		for ( TINT i = 0; i < *TREINTERPRETCAST( TINT*, TSTATICCAST( TUINT8, m_pIOData ) + 4 ); i++ )
		{
			IOInput* pInput = *TREINTERPRETCAST( IOInput**, TSTATICCAST( TUINT8, m_pIOData ) + 8 ) + i;

			if ( pInput->iMessage == a_pMessage[ 0 ] && CanExecuteActionInvoker( pInput, a_pOther ) )
				ExecuteActionInvoker( pInput, a_pOther );
		}
	}
}

// $deBlob: FUNCTION 00784c80
void AIOEntity::SendInputMessage( TINT a_iMessage, ABaseEntity* a_pOther )
{
	TINT aMessage[ 3 ] = { a_iMessage, 0, 0 };
	HandleInputMessage( aMessage, a_pOther );
}

// $deBlob: FUNCTION 00783c70
void AIOEntity::SetPosition( const TVector4& a_rPosition )
{
	m_Flags |= FLAG_UpdateMatrix;
	m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rPosition;

	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOCollisionComponent::TYPE )
		{
			AIOCollisionComponent* pCollision = TSTATICCAST( AIOCollisionComponent, pComponent );
			pCollision->UpdateWorldTransform( m_oMatrix );

			TVector4 vScale;
			pCollision->GetScale( vScale );

			TMatrix44 matCollision;
			pCollision->GetMatrix( matCollision );

			GrowBBox( matCollision, vScale );
			return;
		}
	}
}

// $deBlob: FUNCTION 00783ce0
void AIOEntity::SetMatrix( const TMatrix44& a_rMatrix )
{
	m_oMatrix = a_rMatrix;
	m_Flags |= FLAG_UpdateMatrix;

	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOCollisionComponent::TYPE )
		{
			AIOCollisionComponent* pCollision = TSTATICCAST( AIOCollisionComponent, pComponent );
			pCollision->UpdateWorldTransform( m_oMatrix );

			TVector4 vScale;
			pCollision->GetScale( vScale );

			TMatrix44 matCollision;
			pCollision->GetMatrix( matCollision );

			GrowBBox( matCollision, vScale );
			return;
		}
	}
}

// $deBlob: FUNCTION 00783d70
TVector4 AIOEntity::GetBasePosition() const
{
	return m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $deBlob: FUNCTION 00783d90
ACollisionObject* AIOEntity::GetCollisionObject()
{
	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOCollisionComponent::TYPE )
			return TSTATICCAST( AIOCollisionComponent, pComponent )->GetCollisionObject();
	}

	return TNULL;
}

// $deBlob: FUNCTION 00783dc0
void* AIOEntity::GetModelComponent()
{
	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOMeshComponent::TYPE )
			return TSTATICCAST( AIOMeshComponent, pComponent )->GetModelComponent();
	}

	return TNULL;
}

// The messages of the other entities: the damage (a_pUnk2's 0x6, 0xB and 0xC lower the hit points by the attack's
// +0x14, 0x2E at 0), the touches (4: the input messages 6 once and 4, 5 for the flag 0x20) and the relayed actions (0x21)
// $deBlob: FUNCTION 00783f20
void AIOEntity::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	TINT iDamage     = -1;
	TINT aInput[ 3 ] = { 0, 0, 0 };

	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_PLAYERINTERACT:
			aInput[ 0 ] = 0x33;
			HandleInputMessage( aInput, a_pCaller );
			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
			if ( ( a_pCaller->GetFlags() & 0x10 ) != 0 )
			{
				if ( ( m_uiFlags & 0x200 ) == 0 )
				{
					m_uiFlags |= 0x200;

					TINT aTouch[ 3 ] = { 6, 0, 0 };
					HandleInputMessage( aTouch, a_pCaller );
				}

				m_uiFlags |= 0x400;
				aInput[ 0 ] = 4;
				HandleInputMessage( aInput, a_pCaller );
			}
			else if ( ( a_pCaller->GetFlags() & 0x20 ) != 0 )
			{
				aInput[ 0 ] = 5;
				HandleInputMessage( aInput, a_pCaller );
			}

			break;
		case AEntityIOPacket::MESSAGE_PLAYERATTACK:
			iDamage = 8;
			break;
		case AEntityIOPacket::MESSAGE_ENEMYATTACK:
			iDamage = 9;
			break;
		case AEntityIOPacket::MESSAGE_PROPATTACK:
			iDamage = 10;
			break;
		case AEntityIOPacket::MESSAGE_IOINPUT:
			aInput[ 0 ] = *TREINTERPRETCAST( TINT*, a_rPacket.m_iData );
			HandleInputMessage( aInput, a_pCaller );
			break;
	}

	if ( iDamage != -1 )
	{
		aInput[ 0 ] = iDamage;
		HandleInputMessage( aInput, a_pCaller );

		TINT iHitPoints = m_iHitPoints - *TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, a_rPacket.m_iData ) + 0x14 );
		m_iHitPoints    = TINT16( iHitPoints );

		if ( iHitPoints <= 0 )
		{
			TINT aDestroyed[ 3 ] = { 0x2E, 0, 0 };
			HandleInputMessage( aDestroyed, a_pCaller );
		}
	}

	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
		pComponent->HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007840c0
void AIOEntity::Reset( ABaseEntity* a_pEntity )
{
	SetMatrix( m_oLocalMatrix );
	SetPosition( m_oLocalMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );

	for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
		pUpdater->Stop();

	TINT aMessage[ 3 ] = { 0x30, 0, 0 };
	HandleInputMessage( aMessage, a_pEntity );
}

// $deBlob: FUNCTION 007841a0
void AIOEntity::SetActive( TBOOL a_bActive )
{
	TINT aMessage[ 3 ] = { 0, 0, 0 };

	if ( a_bActive )
	{
		if ( ( m_uiFlags & 0x10 ) != 0 )
			return;

		m_uiFlags |= 0x10;
		aMessage[ 0 ] = 2;
	}
	else
	{
		if ( ( m_uiFlags & 0x10 ) == 0 )
			return;

		m_uiFlags &= ~0x10;
		aMessage[ 0 ] = 3;
	}

	HandleInputMessage( aMessage, this );
}

// $deBlob: FUNCTION 00784200
void AIOEntity::SetCounter( TINT a_iCounter )
{
	TINT aEqual[ 3 ]   = { 0x14, 0, 0 };
	m_iCounter         = a_iCounter;
	TINT aGreater[ 3 ] = { 0x16, 0, 0 };
	TINT aLess[ 3 ]    = { 0x15, 0, 0 };

	HandleInputMessage( aEqual, this );
	HandleInputMessage( aGreater, this );
	HandleInputMessage( aLess, this );
}

// $deBlob: FUNCTION 00784280
void AIOEntity::SetIOFlags( TUINT32 a_uiFlags, TBOOL a_bSet )
{
	if ( a_bSet )
		m_uiFlags |= a_uiFlags;
	else
		m_uiFlags &= ~a_uiFlags;
}

// $deBlob: FUNCTION 007842b0
TBOOL AIOEntity::HasIOFlags( TUINT32 a_uiFlags ) const
{
	return ( m_uiFlags & a_uiFlags ) != 0;
}

// $deBlob: FUNCTION 007842d0
void AIOEntity::FXSpawn( TINT a_iID, ABaseEntity* a_pAt )
{
	for ( FXComposition* pFX = m_pF8List; pFX != TNULL; pFX = pFX->m_pNext )
	{
		if ( pFX->m_iID == a_iID )
		{
			pFX->m_pFollowedEntity = a_pAt;
			pFX->m_oInstance.TriggerEffect( a_pAt->GetMatrix() );
			SetTicking( TTRUE );
		}
	}
}

// $deBlob: FUNCTION 00784330
void AIOEntity::FXStop( TINT a_iID, TBOOL a_bInstant )
{
	for ( FXComposition* pFX = m_pF8List; pFX != TNULL; pFX = pFX->m_pNext )
	{
		if ( pFX->m_iID == a_iID )
		{
			if ( a_bInstant )
				pFX->m_oInstance.StopInstant();
			else
				pFX->m_oInstance.Stop();
		}
	}
}

// The action 0x3B: a duration starts the action's IORotateUpdater, none rotates the entity at once by the angle (in
// degrees, around X, Z or Y), or to the angle (keeping the other two Euler angles) when the parameter at +0x10 is set
// $deBlob: FUNCTION 00784370
void AIOEntity::Rotate( IOAction* a_pAction )
{
	const TFLOAT* pParams  = TSTATICCAST( TFLOAT, a_pAction->pParams );
	const TINT*   piParams = TSTATICCAST( TINT, a_pAction->pParams );

	if ( pParams[ 0 ] != 0.0f )
	{
		for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
		{
			if ( pUpdater->GetAction() == a_pAction )
			{
				pUpdater->Activate();
				SetTicking( TTRUE );
				return;
			}
		}

		return;
	}

	TMatrix44 matRotation = m_oMatrix;

	if ( piParams[ 4 ] == 0 )
	{
		switch ( piParams[ 3 ] )
		{
			case 0:
				matRotation.RotateX( pParams[ 1 ] * 0.017453292f );
				break;
			case 1:
				matRotation.RotateZ( pParams[ 1 ] * 0.017453292f );
				break;
			case 2:
				matRotation.RotateY( pParams[ 1 ] * 0.017453292f );
				break;
		}

		SetMatrix( matRotation );
		return;
	}

	TVector4 vTranslation = m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	TVector3 vEuler;
	matRotation.GetEulerXYZ( vEuler );
	matRotation.Identity();

	switch ( piParams[ 3 ] )
	{
		case 0:
			matRotation.RotateX( pParams[ 1 ] * 0.017453292f );
			matRotation.RotateY( vEuler.y );
			matRotation.RotateZ( vEuler.z );
			break;
		case 1:
			matRotation.RotateX( vEuler.x );
			matRotation.RotateY( vEuler.y );
			matRotation.RotateZ( pParams[ 1 ] * 0.017453292f );
			break;
		case 2:
			matRotation.RotateX( vEuler.x );
			matRotation.RotateY( pParams[ 1 ] * 0.017453292f );
			matRotation.RotateZ( vEuler.z );
			break;
	}

	matRotation.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vTranslation;
	SetMatrix( matRotation );
}

// The action 0x3C: a duration starts the action's IOMoveUpdater, none moves the entity at once to its first target
// (the parameter at +0x20 is 1) or by the offset (the data is Z up) from its position or its start position (+0x10)
// $deBlob: FUNCTION 00784580
void AIOEntity::Move( IOAction* a_pAction, IOInput* a_pInput )
{
	const TFLOAT* pParams  = TSTATICCAST( TFLOAT, a_pAction->pParams );
	const TINT*   piParams = TSTATICCAST( TINT, a_pAction->pParams );

	if ( pParams[ 0 ] == 0.0f )
	{
		if ( piParams[ 8 ] == 1 )
		{
			ABaseEntity* pTarget = GetMessageTarget( a_pInput, a_pAction, 0 );

			if ( pTarget != TNULL )
				SetPosition( pTarget->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		}
		else
		{
			TVector4 vPosition = ( piParams[ 4 ] != 0 ) ? m_oLocalMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) : m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
			vPosition.x        = pParams[ 1 ] + vPosition.x;
			vPosition.y        = vPosition.y - pParams[ 3 ];
			vPosition.z        = pParams[ 2 ] + vPosition.z;
			SetPosition( vPosition );
		}

		return;
	}

	for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
	{
		if ( pUpdater->GetAction() == a_pAction )
		{
			pUpdater->Activate();
			SetTicking( TTRUE );
			return;
		}
	}
}

// Stops the rotations (the updaters of the actions 0x3B)
// $deBlob: FUNCTION 00784660
void AIOEntity::StopRotations()
{
	for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
	{
		IOAction* pAction = TSTATICCAST( IOAction, pUpdater->GetAction() );

		if ( pAction != TNULL && pAction->iType == 0x3B )
			pUpdater->Stop();
	}
}

// The action 0x27: starts the action's IOShakeUpdater, a_pShaker is the entity that shakes this one
// $deBlob: FUNCTION 00784690
void AIOEntity::StartShake( IOAction* a_pAction, ABaseEntity* a_pShaker )
{
	for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
	{
		if ( pUpdater->GetAction() == a_pAction )
		{
			TSTATICCAST( IOShakeUpdater, pUpdater )->SetShaker( a_pShaker );
			pUpdater->Activate();
			SetTicking( TTRUE );
			return;
		}
	}
}

// The action 0x3E: a duration starts the action's IOFadeUpdater from the model's alpha, none sets the alpha at once
// $deBlob: FUNCTION 007846d0
void AIOEntity::FadeMesh( IOAction* a_pAction )
{
	if ( GetModelComponent() == TNULL )
		return;

	const TFLOAT* pParams = TSTATICCAST( TFLOAT, a_pAction->pParams );

	if ( pParams[ 0 ] == 0.0f )
	{
		TFLOAT fAlpha = pParams[ 1 ];

		if ( GetModelComponent() != TNULL )
		{
			TModelInstance* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();
			pModelInstance->SetForceFade( TTRUE );
			pModelInstance->SetFade( fAlpha );
		}

		return;
	}

	for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
	{
		if ( pUpdater->GetAction() == a_pAction )
		{
			TSTATICCAST( IOFadeUpdater, pUpdater )->SetStartAlpha( TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance()->GetFade() );
			pUpdater->Activate();
			pUpdater->SetTime( pParams[ 0 ] );
			SetTicking( TTRUE );
			return;
		}
	}
}

// $deBlob: FUNCTION 00784790
void AIOEntity::SetAlpha( TFLOAT a_fAlpha )
{
	if ( GetModelComponent() != TNULL )
	{
		TModelInstance* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();
		pModelInstance->SetForceFade( TTRUE );
		pModelInstance->SetFade( a_fAlpha );
	}
}

// The action 0x3F: a duration starts the action's IOScaleUpdater from the model's scale, none scales at once
// $deBlob: FUNCTION 007847d0
void AIOEntity::Scale( IOAction* a_pAction )
{
	const TFLOAT* pParams  = TSTATICCAST( TFLOAT, a_pAction->pParams );
	const TINT*   piParams = TSTATICCAST( TINT, a_pAction->pParams );

	if ( pParams[ 0 ] == 0.0f )
	{
		TVector4 vScale( pParams[ 1 ], pParams[ 2 ], pParams[ 3 ], 1.0f );

		if ( GetModelComponent() != TNULL )
			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetScale( vScale );

		SetBBox( vScale );

		if ( piParams[ 4 ] != 0 )
		{
			AIOEntityComponent* pComponent = m_pFCList;

			while ( pComponent != TNULL && pComponent->GetType() != AIOCollisionComponent::TYPE )
				pComponent = pComponent->GetNext();

			if ( GetCollisionObject() != TNULL )
			{
				AIOCollisionComponent* pCollision = TSTATICCAST( AIOCollisionComponent, pComponent );
				pCollision->SetCollisionDirty( vScale );

				TMatrix44 matCollision;
				pCollision->GetMatrix( matCollision );
				GrowBBox( matCollision, vScale );
			}
		}

		return;
	}

	for ( IIOMessageUpdater* pUpdater = m_p100List; pUpdater != TNULL; pUpdater = pUpdater->GetNext() )
	{
		if ( pUpdater->GetAction() == a_pAction )
		{
			TVector4 vStartScale = ( GetModelComponent() != TNULL ) ? TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetScale() : TVector4( 1.0f, 1.0f, 1.0f, 1.0f );

			TSTATICCAST( IOScaleUpdater, pUpdater )->SetStartScale( vStartScale );
			pUpdater->Activate();
			pUpdater->SetTime( pParams[ 0 ] );
			SetTicking( TTRUE );
			return;
		}
	}
}

// $deBlob: FUNCTION 00784910
void AIOEntity::SetScale( const TVector4& a_rScale, TBOOL a_bScaleCollision )
{
	if ( GetModelComponent() != TNULL )
		TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetScale( a_rScale );

	SetBBox( a_rScale );

	if ( a_bScaleCollision )
	{
		for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
		{
			if ( pComponent->GetType() == AIOCollisionComponent::TYPE )
			{
				AIOCollisionComponent* pCollision = TSTATICCAST( AIOCollisionComponent, pComponent );
				pCollision->SetCollisionDirty( a_rScale );

				TMatrix44 matCollision;
				pCollision->GetMatrix( matCollision );
				GrowBBox( matCollision, a_rScale );
				return;
			}
		}
	}
}

// $deBlob: FUNCTION 007849a0
void AIOEntity::PlayAnim( IOAction* a_pAction )
{
	if ( GetModelComponent() == TNULL )
		return;

	// The anim IDs: the ID (TUINT16), the action (+4), the next (+8)
	for ( TUINT8* pNode = TSTATICCAST( TUINT8, m_pCCList ); pNode != TNULL; pNode = *TREINTERPRETCAST( TUINT8**, pNode + 8 ) )
	{
		if ( *TREINTERPRETCAST( IOAction**, pNode + 4 ) == a_pAction )
		{
			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->AddAnim( *TREINTERPRETCAST( TUINT16*, pNode ), 0.15f, 1.0f, 1.0f );
			return;
		}
	}
}

// NOTE: the same search as FindTargetList
// $deBlob: FUNCTION 00784af0
ABaseEntity* AIOEntity::GetMessageTarget( void* a_pInput, void* a_pAction, TINT a_iIndex )
{
	TargetList* pTargets = FindTargetList( a_pInput, a_pAction );

	if ( pTargets == TNULL || a_iIndex >= pTargets->iNumTargets )
		return TNULL;

	return pTargets->pTargets[ a_iIndex ].pEntity;
}

// $deBlob: FUNCTION 00784cb0
void AIOEntity::Destroy()
{
	SetVisible( TFALSE );
	SetHash( TFALSE );
	RemoveFromPhysics();
}

// The bounding box (its half extents) grows to the eight corners of the collision's box: the axes of a_rMatrix scaled
// by a_rScale, around its translation relative to the entity
// $deBlob: FUNCTION 00784cd0
void AIOEntity::GrowBBox( const TMatrix44& a_rMatrix, const TVector4& a_rScale )
{
	const TVector4& vTranslation = a_rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	const TVector4& vPosition    = m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

	TVector4 vOffset( vTranslation.x - vPosition.x, vTranslation.y - vPosition.y, vTranslation.z - vPosition.z, vTranslation.w );

	TVector4 vAxisX;
	vAxisX.Multiply( a_rMatrix.AsBasisVector4( BASISVECTOR_RIGHT ), a_rScale.x );
	TVector4 vAxisY;
	vAxisY.Multiply( a_rMatrix.AsBasisVector4( BASISVECTOR_UP ), a_rScale.y );
	TVector4 vAxisZ;
	vAxisZ.Multiply( a_rMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), a_rScale.z );

	TVector4 vExtents = m_vBBox;

	// The corners in this order: +X+Y+Z, +X+Y-Z, +X-Y+Z, -X+Y+Z, +X-Y-Z, -X+Y-Z, -X-Y+Z, -X-Y-Z
	static TCONSTEXPR_VAR TFLOAT s_aSigns[ 8 ][ 3 ] = {
		{ 1.0f, 1.0f, 1.0f },
		{ 1.0f, 1.0f, -1.0f },
		{ 1.0f, -1.0f, 1.0f },
		{ -1.0f, 1.0f, 1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{ -1.0f, 1.0f, -1.0f },
		{ -1.0f, -1.0f, 1.0f },
		{ -1.0f, -1.0f, -1.0f },
	};

	for ( TINT i = 0; i < 8; i++ )
	{
		TFLOAT fX = vOffset.x + s_aSigns[ i ][ 0 ] * vAxisX.x + s_aSigns[ i ][ 1 ] * vAxisY.x + s_aSigns[ i ][ 2 ] * vAxisZ.x;
		TFLOAT fY = vOffset.y + s_aSigns[ i ][ 0 ] * vAxisX.y + s_aSigns[ i ][ 1 ] * vAxisY.y + s_aSigns[ i ][ 2 ] * vAxisZ.y;
		TFLOAT fZ = vOffset.z + s_aSigns[ i ][ 0 ] * vAxisX.z + s_aSigns[ i ][ 1 ] * vAxisY.z + s_aSigns[ i ][ 2 ] * vAxisZ.z;

		if ( vExtents.x < TMath::Abs( fX ) ) vExtents.x = TMath::Abs( fX );
		if ( vExtents.y < TMath::Abs( fY ) ) vExtents.y = TMath::Abs( fY );
		if ( vExtents.z < TMath::Abs( fZ ) ) vExtents.z = TMath::Abs( fZ );
	}

	SetBBox( vExtents );
}

// $deBlob: FUNCTION 00785270
TVector4 AIOEntity::GetReactionForce( void* a_pUnk1, void* a_pUnk2 )
{
	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOCollisionComponent::TYPE )
			return TSTATICCAST( AIOCollisionComponent, pComponent )->GetReactionForce( *TSTATICCAST( const TVector4, a_pUnk1 ), TSTATICCAST( ABaseEntity, a_pUnk2 ) );
	}

	return TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 007852d0
TVector4 AIOEntity::GetReactionRotation( void* a_pUnk )
{
	for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
	{
		if ( pComponent->GetType() == AIOCollisionComponent::TYPE )
		{
			TQuaternion qRotation = TSTATICCAST( AIOCollisionComponent, pComponent )->GetReactionRotation( TSTATICCAST( ABaseEntity, a_pUnk ) );
			return TVector4( qRotation.x, qRotation.y, qRotation.z, qRotation.w );
		}
	}

	return TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
}

// An input runs unless it cools down, its condition on the IO flags (m_uiFlags' bits 0..3) fails or, for the counter
// messages 0x14 (equal), 0x15 (less) and 0x16 (greater), the counter doesn't match the input's value
// $deBlob: FUNCTION 00785330
TBOOL AIOEntity::CanExecuteActionInvoker( IOInput* a_pInput, ABaseEntity* a_pOther )
{
	for ( TINT i = 0; i < m_iDCCount; i++ )
	{
		if ( m_pE0Items[ i ].m_pInput == a_pInput )
			return TFALSE;
	}

	if ( a_pInput->iCondition == 1 )
	{
		// The flags each condition value needs
		TUINT32 uiMask;

		switch ( a_pInput->iConditionValue )
		{
			case 0x0: uiMask = 0x1; break;
			case 0x1: uiMask = 0x2; break;
			case 0x2: uiMask = 0x4; break;
			case 0x3: uiMask = 0x8; break;
			case 0x4: uiMask = 0x3; break;
			case 0x5: uiMask = 0x7; break;
			case 0x6: uiMask = 0xB; break;
			case 0x7: uiMask = 0x5; break;
			case 0x8: uiMask = 0xD; break;
			case 0x9: uiMask = 0x9; break;
			case 0xA: uiMask = 0x6; break;
			case 0xB: uiMask = 0xE; break;
			case 0xC: uiMask = 0xA; break;
			case 0xD: uiMask = 0xC; break;
			case 0xE: uiMask = 0xF; break;
			default: return TFALSE;
		}

		if ( ( m_uiFlags & uiMask ) != uiMask )
			return TFALSE;
	}

	if ( a_pInput->iMessage == 0x14 )
	{
		if ( a_pInput->pParams[ 0 ] != m_iCounter )
			return TFALSE;
	}
	else if ( a_pInput->iMessage == 0x15 )
	{
		if ( m_iCounter >= a_pInput->pParams[ 0 ] )
			return TFALSE;
	}
	else if ( a_pInput->iMessage == 0x16 && m_iCounter <= a_pInput->pParams[ 0 ] )
	{
		return TFALSE;
	}

	return TTRUE;
}

// Runs the input: its cooldown starts, the messages 0 and 0x30 apply their parameters, then each action runs (its
// callback or the components) or waits for its delay (ActionDelay)
// $deBlob: FUNCTION 007854e0
void AIOEntity::ExecuteActionInvoker( IOInput* a_pInput, ABaseEntity* a_pOther )
{
	if ( a_pInput->fCooldown > 0.0f )
	{
		for ( TINT i = 0; i < m_iDCCount; i++ )
		{
			if ( !( m_pE0Items[ i ].m_fTimer > 0.0f ) )
			{
				m_pE0Items[ i ].m_fTimer = a_pInput->fCooldown;
				m_pE0Items[ i ].m_pInput = a_pInput;
				break;
			}
		}

		SetTicking( TTRUE );
	}

	if ( a_pInput->iMessage == 0 )
		ApplyStartParams( TREINTERPRETCAST( const StartParams*, a_pInput->pParams ) );
	else if ( a_pInput->iMessage == 0x30 )
		ApplyResetParams( TREINTERPRETCAST( const StartParams*, a_pInput->pParams ) );

	for ( TINT i = 0; i < a_pInput->iNumActions; i++ )
	{
		IOAction* pAction = &a_pInput->pActions[ i ];

		if ( pAction->fDelay <= 0.0f )
		{
			t_IOCallback fnCallback = sm_aIOCallbacks[ pAction->iType ];

			if ( fnCallback != TNULL )
			{
				fnCallback( pAction, a_pInput, this, a_pOther );
			}
			else
			{
				for ( AIOEntityComponent* pComponent = m_pFCList; pComponent != TNULL; pComponent = pComponent->GetNext() )
				{
					if ( pComponent->ExecuteActionInvoker( TREINTERPRETCAST( TINT*, pAction ), this, a_pOther ) == TTRUE )
						break;
				}
			}
		}
		else
		{
			for ( TINT j = 0; j < m_iD4Count; j++ )
			{
				if ( !( m_pD8Items[ j ].m_fTimer > 0.0f ) )
				{
					m_pD8Items[ j ].m_pAction = pAction;
					m_pD8Items[ j ].m_fTimer  = pAction->fDelay;
					m_pD8Items[ j ].m_pOther  = a_pOther;
					m_pD8Items[ j ].m_pInput  = a_pInput;
					break;
				}
			}

			SetTicking( TTRUE );
		}
	}
}

// The message 0 (the IO's start)
// $deBlob: FUNCTION 00785860
void AIOEntity::ApplyStartParams( const StartParams* a_pParams )
{
	m_iHitPoints = TINT16( a_pParams->iHitPoints );
	m_iCounter   = TINT16( a_pParams->iCounter );
	SetVisible( a_pParams->bVisible != 0 );
	SetActive( a_pParams->bActive != 0 );
	m_fHeartBeat = a_pParams->fHeartBeat;
	m_uiColour   = a_pParams->iBlue | ( ( a_pParams->iGreen | ( ( a_pParams->iRed | 0xFFFFFF00 ) << 8 ) ) << 8 );
}

// The message 0x30 (Reset)
// $deBlob: FUNCTION 007858d0
void AIOEntity::ApplyResetParams( const StartParams* a_pParams )
{
	m_iHitPoints = TINT16( a_pParams->iHitPoints );
	m_iCounter   = TINT16( a_pParams->iCounter );
	SetVisible( a_pParams->bVisible != 0 );
	SetActive( a_pParams->bActive != 0 );
	m_fHeartBeat = a_pParams->fHeartBeat;
}

// A random int from a_iLower to a_iUpper (excluded) of the game's random generator
// $deBlob: FUNCTION 007e4f90
static TINT IOGetRandomInt( TINT a_iLower, TINT a_iUpper )
{
	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	if ( a_iLower == a_iUpper )
		return a_iLower;

	TASSERT( a_iUpper > a_iLower );
	return a_iLower + rRandom.GetInt( a_iUpper - a_iLower );
}

//-----------------------------------------------------------------------------
// IOCallbacks
//-----------------------------------------------------------------------------

// The parameters of an action
template <class T>
static T* IOGetParams( void* a_pAction )
{
	return TSTATICCAST( T, TSTATICCAST( AIOEntity::IOAction, a_pAction )->pParams );
}

// $deBlob: FUNCTION 00783390
void AIOEntity::InitialiseClass()
{
	TUtil::MemClear( sm_aIOCallbacks, sizeof( sm_aIOCallbacks ) );
	sm_aIOCallbacks[ 0x27 ] = IOCallbacks::OnShake;
	sm_aIOCallbacks[ 0x2A ] = IOCallbacks::OnActivate;
	sm_aIOCallbacks[ 0x2B ] = IOCallbacks::OnDeactivate;
	sm_aIOCallbacks[ 0x19 ] = IOCallbacks::OnSetVisible;
	sm_aIOCallbacks[ 0x26 ] = IOCallbacks::OnDamage;
	sm_aIOCallbacks[ 0x20 ] = IOCallbacks::OnScreenFadeIn;
	sm_aIOCallbacks[ 0x1F ] = IOCallbacks::OnScreenFadeOut;
	sm_aIOCallbacks[ 0x0C ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x0D ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x0E ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x0F ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x4A ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x4B ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x4C ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x4D ] = IOCallbacks::RelayMessage;
	sm_aIOCallbacks[ 0x31 ] = IOCallbacks::OnSetHeartBeat;
	sm_aIOCallbacks[ 0x2F ] = IOCallbacks::OnReset;
	sm_aIOCallbacks[ 0x2D ] = IOCallbacks::OnSetHitPoints;
	sm_aIOCallbacks[ 0x29 ] = IOCallbacks::OnFindNewTarget;
	sm_aIOCallbacks[ 0x24 ] = IOCallbacks::OnEndLevel;
	sm_aIOCallbacks[ 0x23 ] = IOCallbacks::OnTeleportCaller;
	sm_aIOCallbacks[ 0x22 ] = IOCallbacks::OnAutosave;
	sm_aIOCallbacks[ 0x1B ] = IOCallbacks::OnPlayAnim;
	sm_aIOCallbacks[ 0x18 ] = IOCallbacks::OnDestroy;
	sm_aIOCallbacks[ 0x17 ] = IOCallbacks::OnSetFlag;
	sm_aIOCallbacks[ 0x13 ] = IOCallbacks::OnModifyCounter;
	sm_aIOCallbacks[ 0x12 ] = IOCallbacks::OnSetCounter;
	sm_aIOCallbacks[ 0x38 ] = AGameStatus::OnCheckPointActivate;
	sm_aIOCallbacks[ 0x39 ] = AGameStatus::OnCheckPointDeactivate;
	sm_aIOCallbacks[ 0x3A ] = AGameStatus::OnCheckPointRegister;
	sm_aIOCallbacks[ 0x35 ] = IOCallbacks::OnKnockBack;
	sm_aIOCallbacks[ 0x37 ] = IOCallbacks::OnBounceCaller;
	sm_aIOCallbacks[ 0x1C ] = IOCallbacks::OnFXSpawn;
	sm_aIOCallbacks[ 0x1D ] = IOCallbacks::OnFXStop;
	sm_aIOCallbacks[ 0x3B ] = IOCallbacks::OnRotate;
	sm_aIOCallbacks[ 0x3C ] = IOCallbacks::OnMove;
	sm_aIOCallbacks[ 0x3D ] = IOCallbacks::OnSetModelFlag;
	sm_aIOCallbacks[ 0x3E ] = IOCallbacks::OnFadeMesh;
	sm_aIOCallbacks[ 0x3F ] = IOCallbacks::OnScaleEntity;
	sm_aIOCallbacks[ 0x41 ] = IOCallbacks::OnFireProjectile;
	sm_aIOCallbacks[ 0x43 ] = IOCallbacks::OnSpawnObject;
	sm_aIOCallbacks[ 0x47 ] = IOCallbacks::OnTriggerTargets;
	sm_aIOCallbacks[ 0x46 ] = IOCallbacks::OnTriggerTargets;
	sm_aIOCallbacks[ 0x45 ] = IOCallbacks::OnTriggerTargets;
	sm_aIOCallbacks[ 0x44 ] = IOCallbacks::OnTriggerTargets;
	sm_aIOCallbacks[ 0x48 ] = IOCallbacks::OnRotationStop;
}

// $deBlob: FUNCTION 007833b0
void AIOEntity::DeinitialiseClass()
{
}

// The entity's last target is an AIOEntity
static AIOEntity* IOGetIOEntity( void* a_pEntity )
{
	ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, a_pEntity );

	if ( pEntity != TNULL && pEntity->IsA( &TGetClass( AIOEntity ) ) )
		return pEntity->CastTo<AIOEntity>();

	return TNULL;
}

// $deBlob: FUNCTION 007c99e0
void __stdcall IOCallbacks::OnShake( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	AIOEntity::IOAction* pAction = TSTATICCAST( AIOEntity::IOAction, a_pAction );
	ABaseEntity*         pOther  = TSTATICCAST( ABaseEntity, a_pOther );

	switch ( IOGetParams<TINT>( a_pAction )[ 0 ] )
	{
		case 0:
			a_pEntity->StartShake( pAction, pOther );
			break;
		case 1:
		{
			AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

			if ( pTargets != TNULL )
			{
				for ( TINT i = 0; i < pTargets->iNumTargets; i++ )
				{
					AIOEntity* pTarget = IOGetIOEntity( pTargets->pTargets[ i ].pEntity );

					if ( pTarget != TNULL )
						pTarget->StartShake( pAction, pOther );
				}
			}

			break;
		}
		case 2:
		{
			AIOEntity* pTarget = IOGetIOEntity( pOther );

			if ( pTarget != TNULL )
				pTarget->StartShake( pAction, pOther );

			break;
		}
		case 4:
		{
			AIOEntity* pTarget = IOGetIOEntity( a_pEntity->m_pLastTarget );

			if ( pTarget != TNULL )
				pTarget->StartShake( pAction, pOther );

			break;
		}
	}
}

// $deBlob: FUNCTION 007c9af0
void __stdcall IOCallbacks::OnDamage( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT* pParams = IOGetParams<TINT>( a_pAction );

	if ( pParams[ 0 ] == 0 )
	{
		TINT iHitPoints         = a_pEntity->m_iHitPoints - pParams[ 1 ];
		a_pEntity->m_iHitPoints = TINT16( iHitPoints );

		if ( iHitPoints <= 0 )
		{
			TINT aMessage[ 3 ] = { 0x2E, 0, 0 };
			a_pEntity->HandleInputMessage( aMessage, TSTATICCAST( ABaseEntity, a_pOther ) );
		}
	}
}

// $deBlob: FUNCTION 007c9b60
void __stdcall IOCallbacks::OnSetVisible( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT*        pParams = IOGetParams<TINT>( a_pAction );
	ABaseEntity* pTarget = TNULL;

	switch ( pParams[ 0 ] )
	{
		case 0:
			pTarget = a_pEntity;
			break;
		case 1:
		{
			AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

			if ( pTargets != TNULL )
			{
				for ( TINT i = 0; i < pTargets->iNumTargets; i++ )
				{
					if ( pTargets->pTargets[ i ].pEntity != TNULL )
						pTargets->pTargets[ i ].pEntity->SetVisible( pParams[ 1 ] != 0 );
				}
			}

			return;
		}
		case 2:
			pTarget = TSTATICCAST( ABaseEntity, a_pOther );
			break;
		case 4:
			pTarget = a_pEntity->m_pLastTarget;
			break;
		default:
			return;
	}

	// NOTE: retail doesn't check the entity itself (case 0) for TNULL
	if ( pParams[ 0 ] == 0 || pTarget != TNULL )
		pTarget->SetVisible( pParams[ 1 ] != 0 );
}

// $deBlob: FUNCTION 007c9c10
void __stdcall IOCallbacks::OnScreenFadeOut( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	AGameStateController::GetSingleton()->FadeToBlack( IOGetParams<TFLOAT>( a_pAction )[ 0 ], 0 );
}

// $deBlob: FUNCTION 007c9c40
void __stdcall IOCallbacks::OnScreenFadeIn( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	AGameStateController::GetSingleton()->FadeFromBlack( IOGetParams<TFLOAT>( a_pAction )[ 0 ], 0 );
}

// The finished messages of the updaters (0xC..0xF, 0x4A..0x4D): the entity gets the action as an input message, its
// targets (one at random with the parameter at +4) or its last target get MESSAGE_IOINPUT with the action's type
// $deBlob: FUNCTION 007c9c70
void __stdcall IOCallbacks::RelayMessage( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	AIOEntity::IOAction* pAction = TSTATICCAST( AIOEntity::IOAction, a_pAction );
	TINT*                pParams = IOGetParams<TINT>( a_pAction );

	if ( pParams[ 0 ] == 0 )
	{
		a_pEntity->HandleInputMessage( TREINTERPRETCAST( TINT*, pAction ), a_pEntity );
	}
	else if ( pParams[ 0 ] == 1 )
	{
		TINT            aType[ 2 ] = { pAction->iType, 0 };
		AEntityIOPacket oPacket    = { AEntityIOPacket::MESSAGE_IOINPUT, TREINTERPRETCAST( TINT, aType ) };

		AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

		if ( pTargets != TNULL )
		{
			if ( pParams[ 1 ] != 0 )
			{
				ABaseEntity* pTarget = pTargets->pTargets[ IOGetRandomInt( 0, pTargets->iNumTargets ) ].pEntity;
				pTarget->HandleEntityIO( a_pEntity, oPacket );
				a_pEntity->m_pLastTarget = pTarget;
			}
			else
			{
				for ( TINT i = 0; i < pTargets->iNumTargets; i++ )
				{
					ABaseEntity* pTarget = pTargets->pTargets[ i ].pEntity;

					if ( pTarget != TNULL )
					{
						pTarget->HandleEntityIO( a_pEntity, oPacket );
						a_pEntity->m_pLastTarget = pTarget;
					}
				}
			}
		}
	}
	else if ( pParams[ 0 ] == 4 )
	{
		ABaseEntity* pTarget = a_pEntity->m_pLastTarget;

		if ( pTarget != TNULL )
		{
			TINT            aType[ 2 ] = { pAction->iType, 0 };
			AEntityIOPacket oPacket    = { AEntityIOPacket::MESSAGE_IOINPUT, TREINTERPRETCAST( TINT, aType ) };
			pTarget->HandleEntityIO( a_pEntity, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007c9d90
void __stdcall IOCallbacks::OnActivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT iMode = IOGetParams<TINT>( a_pAction )[ 0 ];

	if ( iMode == 0 )
	{
		a_pEntity->SetActive( TTRUE );
	}
	else if ( iMode == 1 )
	{
		AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

		if ( pTargets != TNULL )
		{
			for ( TINT i = 0; i < pTargets->iNumTargets; i++ )
			{
				if ( pTargets->pTargets[ i ].pEntity != TNULL )
					pTargets->pTargets[ i ].pEntity->SetActive( TTRUE );
			}
		}
	}
	else if ( iMode == 4 )
	{
		if ( a_pEntity->m_pLastTarget != TNULL )
			a_pEntity->m_pLastTarget->SetActive( TTRUE );
	}
}

// $deBlob: FUNCTION 007c9e20
void __stdcall IOCallbacks::OnDeactivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT iMode = IOGetParams<TINT>( a_pAction )[ 0 ];

	if ( iMode == 0 )
	{
		a_pEntity->SetActive( TFALSE );
	}
	else if ( iMode == 1 )
	{
		AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

		if ( pTargets != TNULL )
		{
			for ( TINT i = 0; i < pTargets->iNumTargets; i++ )
			{
				if ( pTargets->pTargets[ i ].pEntity != TNULL )
					pTargets->pTargets[ i ].pEntity->SetActive( TFALSE );
			}
		}
	}
	else if ( iMode == 4 )
	{
		if ( a_pEntity->m_pLastTarget != TNULL )
			a_pEntity->m_pLastTarget->SetActive( TFALSE );
	}
}

// $deBlob: FUNCTION 007c9eb0
void __stdcall IOCallbacks::OnEndLevel( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();

	if ( pState->IsA( &TGetClass( AInGameState ) ) == TTRUE )
		pState->CastTo<AInGameState>()->SetLevelEnded( TTRUE );
}

// $deBlob: FUNCTION 007c9f30
void __stdcall IOCallbacks::OnSetHeartBeat( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->m_fHeartBeat = IOGetParams<TFLOAT>( a_pAction )[ 0 ];
}

// $deBlob: FUNCTION 007c9f50
void __stdcall IOCallbacks::OnReset( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT iMode = IOGetParams<TINT>( a_pAction )[ 0 ];

	if ( iMode == 0 )
	{
		a_pEntity->Reset( a_pEntity );
	}
	else if ( iMode == 1 )
	{
		AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

		if ( pTargets != TNULL )
		{
			for ( TINT i = 0; i < pTargets->iNumTargets; i++ )
			{
				if ( pTargets->pTargets[ i ].pEntity != TNULL )
					pTargets->pTargets[ i ].pEntity->Reset( a_pEntity );
			}
		}
	}
	else if ( iMode == 4 && a_pEntity->m_pLastTarget != TNULL )
	{
		a_pEntity->m_pLastTarget->Reset( a_pEntity );
	}
}

// $deBlob: FUNCTION 007c9fe0
void __stdcall IOCallbacks::OnSetHitPoints( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->m_iHitPoints = TINT16( IOGetParams<TINT>( a_pAction )[ 1 ] );
}

// Puts the other entity at the matrix of the target (the entity, its first target or its last target), or leaves it
// at its own matrix
// $deBlob: FUNCTION 007ca010
void __stdcall IOCallbacks::OnTeleportCaller( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	ABaseEntity* pOther = TSTATICCAST( ABaseEntity, a_pOther );

	if ( pOther == TNULL )
		return;

	TMatrix44    matTarget = pOther->GetMatrix();
	ABaseEntity* pTarget   = TNULL;

	switch ( IOGetParams<TINT>( a_pAction )[ 0 ] )
	{
		case 0:
			pTarget = a_pEntity;
			break;
		case 1:
			pTarget = a_pEntity->GetMessageTarget( a_pInvoker, a_pAction, 0 );
			break;
		case 4:
			pTarget = a_pEntity->m_pLastTarget;
			break;
	}

	if ( pTarget != TNULL )
		matTarget = pTarget->GetMatrix();

	pOther->SetMatrix( matTarget );
}

// $deBlob: FUNCTION 007ca0c0
void __stdcall IOCallbacks::OnAutosave( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	if ( AAutosaveManager::GetSingleton() != TNULL )
		AAutosaveManager::GetSingleton()->Save();
}

// $deBlob: FUNCTION 007ca0e0
void __stdcall IOCallbacks::OnPlayAnim( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->PlayAnim( TSTATICCAST( AIOEntity::IOAction, a_pAction ) );
}

// $deBlob: FUNCTION 007ca100
void __stdcall IOCallbacks::OnDestroy( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT iMode = IOGetParams<TINT>( a_pAction )[ 0 ];

	if ( iMode == 0 )
	{
		a_pEntity->Destroy();
	}
	else if ( iMode == 1 )
	{
		ABaseEntity* pTarget   = a_pEntity->GetMessageTarget( a_pInvoker, a_pAction, 0 );
		AIOEntity*   pIOTarget = IOGetIOEntity( pTarget );

		if ( pIOTarget != TNULL )
			pIOTarget->Destroy();

		a_pEntity->m_pLastTarget = pTarget;
	}
	else if ( iMode == 4 )
	{
		AIOEntity* pIOTarget = IOGetIOEntity( a_pEntity->m_pLastTarget );

		if ( pIOTarget != TNULL )
			pIOTarget->Destroy();
	}
}

// $deBlob: FUNCTION 007ca1b0
void __stdcall IOCallbacks::OnSetFlag( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT* pParams = IOGetParams<TINT>( a_pAction );

	if ( pParams[ 0 ] < 0 || pParams[ 0 ] > 3 )
		return;

	TUINT32 uiFlag = 1 << pParams[ 0 ];

	if ( pParams[ 1 ] == 0 )
		a_pEntity->m_uiFlags &= ~uiFlag;
	else
		a_pEntity->m_uiFlags |= uiFlag;
}

// $deBlob: FUNCTION 007ca260
void __stdcall IOCallbacks::OnModifyCounter( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->SetCounter( *IOGetParams<TINT>( a_pAction ) + a_pEntity->m_iCounter );
}

// $deBlob: FUNCTION 007ca280
void __stdcall IOCallbacks::OnSetCounter( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->SetCounter( IOGetParams<TINT>( a_pAction )[ 0 ] );
}

// Knocks the other entity back (an attack MESSAGE_PROPATTACK with the action's two parameters): away from the entity when
// the entity's up axis is +Y, along that axis otherwise
// $deBlob: FUNCTION 007ca2a0
void __stdcall IOCallbacks::OnKnockBack( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	ABaseEntity* pOther = TSTATICCAST( ABaseEntity, a_pOther );

	if ( pOther == TNULL )
		return;

	TUINT*          pParams = IOGetParams<TUINT>( a_pAction );
	const TVector4& vUp     = a_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
	TVector4        vDirection;

	if ( vUp.y * TVector4::VEC_POSY.y + vUp.x * TVector4::VEC_POSY.x + vUp.z * TVector4::VEC_POSY.z == 1.0f )
	{
		const TVector4& vOtherPosition = pOther->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
		const TVector4& vPosition      = a_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
		vDirection.Set( vOtherPosition.x - vPosition.x, vOtherPosition.y - vPosition.y, vOtherPosition.z - vPosition.z, vOtherPosition.w );
	}
	else
	{
		vDirection.Set( vUp.x, vUp.y, vUp.z, 1.0f );
	}

	vDirection.Normalise();

	AAttackMessage oAttack( AAttackMessage::ATTACKTYPE( 0 ), pOther->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), pParams[ 1 ], pParams[ 0 ], vDirection, TFALSE );

	AEntityIOPacket oPacket = { AEntityIOPacket::MESSAGE_PROPATTACK, TREINTERPRETCAST( TINT, &oAttack ) };
	pOther->HandleEntityIO( a_pEntity, oPacket );
}

// $deBlob: FUNCTION 007ca3a0
void __stdcall IOCallbacks::OnBounceCaller( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	ABaseEntity* pOther = TSTATICCAST( ABaseEntity, a_pOther );

	if ( pOther != TNULL )
	{
		AEntityIOPacket oPacket = { AEntityIOPacket::MESSAGE_BOUNCE, 0 };
		pOther->HandleEntityIO( a_pEntity, oPacket );
	}
}

// Triggers the FX compositions of the id at +0xC at the target (one at random with the parameter at +4)
// $deBlob: FUNCTION 007ca3d0
void __stdcall IOCallbacks::OnFXSpawn( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT* pParams = IOGetParams<TINT>( a_pAction );

	switch ( pParams[ 0 ] )
	{
		case 0:
			a_pEntity->FXSpawn( pParams[ 3 ], a_pEntity );
			break;
		case 1:
		{
			ABaseEntity* pTarget = TNULL;

			if ( pParams[ 1 ] != 0 )
			{
				AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

				if ( pTargets != TNULL )
					pTarget = pTargets->pTargets[ IOGetRandomInt( 0, pTargets->iNumTargets ) ].pEntity;
			}
			else
			{
				pTarget = a_pEntity->GetMessageTarget( a_pInvoker, a_pAction, 0 );
			}

			if ( pTarget != TNULL )
				a_pEntity->FXSpawn( pParams[ 3 ], pTarget );

			a_pEntity->m_pLastTarget = pTarget;
			break;
		}
		case 2:
			a_pEntity->FXSpawn( pParams[ 3 ], TSTATICCAST( ABaseEntity, a_pOther ) );
			break;
		case 4:
			if ( a_pEntity->m_pLastTarget != TNULL )
				a_pEntity->FXSpawn( pParams[ 3 ], a_pEntity->m_pLastTarget );

			break;
	}
}

// $deBlob: FUNCTION 007ca4a0
void __stdcall IOCallbacks::OnFXStop( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT* pParams = IOGetParams<TINT>( a_pAction );
	a_pEntity->FXStop( pParams[ 0 ], pParams[ 1 ] != 0 );
}

// $deBlob: FUNCTION 007ca4d0
void __stdcall IOCallbacks::OnRotate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->Rotate( TSTATICCAST( AIOEntity::IOAction, a_pAction ) );
}

// $deBlob: FUNCTION 007ca4f0
void __stdcall IOCallbacks::OnMove( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->Move( TSTATICCAST( AIOEntity::IOAction, a_pAction ), TSTATICCAST( AIOEntity::IOInput, a_pInvoker ) );
}

// The model's flag 0x100
// $deBlob: FUNCTION 007ca510
void __stdcall IOCallbacks::OnSetModelFlag( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	if ( a_pEntity->GetModelComponent() == TNULL )
		return;

	if ( IOGetParams<TINT>( a_pAction )[ 0 ] == 0 )
		TSTATICCAST( AEntityModelComponent, a_pEntity->GetModelComponent() )->GetModelInstance()->SetFlag( AModelInstance2::FLAG( 0x100 ), TFALSE );
	else
		TSTATICCAST( AEntityModelComponent, a_pEntity->GetModelComponent() )->GetModelInstance()->SetFlag( AModelInstance2::FLAG( 0x100 ), TTRUE );
}

// $deBlob: FUNCTION 007ca560
void __stdcall IOCallbacks::OnFadeMesh( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->FadeMesh( TSTATICCAST( AIOEntity::IOAction, a_pAction ) );
}

// $deBlob: FUNCTION 007ca580
void __stdcall IOCallbacks::OnScaleEntity( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->Scale( TSTATICCAST( AIOEntity::IOAction, a_pAction ) );
}

// Fires a projectile of the type at +0 from the entity along its right axis, at the target's position when there's
// one (the target mode at +0x20, a random target with +0x30); +4 makes the parameters the projectile's override
// $deBlob: FUNCTION 007ca5a0
void __stdcall IOCallbacks::OnFireProjectile( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT*        pParams = IOGetParams<TINT>( a_pAction );
	TVector4     vTarget( 0.0f, 0.0f, 0.0f, 1.0f );
	ABaseEntity* pOther = TSTATICCAST( ABaseEntity, a_pOther );

	switch ( pParams[ 8 ] )
	{
		case 1:
			if ( pParams[ 0xC ] == 0 )
			{
				pOther = a_pEntity->GetMessageTarget( a_pInvoker, a_pAction, 0 );

				if ( pOther != TNULL )
					vTarget = pOther->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

				a_pEntity->m_pLastTarget = pOther;
			}
			else
			{
				AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

				if ( pTargets != TNULL )
				{
					pOther                   = pTargets->pTargets[ IOGetRandomInt( 0, pTargets->iNumTargets ) ].pEntity;
					vTarget                  = pOther->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
					a_pEntity->m_pLastTarget = pOther;
				}
			}

			break;
		case 2:
			vTarget                  = pOther->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
			a_pEntity->m_pLastTarget = pOther;
			break;
		case 4:
			if ( a_pEntity->m_pLastTarget != TNULL )
				vTarget = a_pEntity->m_pLastTarget->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

			break;
	}

	AProjectile* pProjectile = AProjectileManager::GetSingleton()->CreateProjectile( AProjectileTypes::PROJECTILETYPES( pParams[ 0 ] ) );

	if ( pProjectile != TNULL )
	{
		if ( pParams[ 1 ] != 0 )
			pProjectile->m_pOverride = TREINTERPRETCAST( AProjectileDataOverride*, pParams );

		TVector4 vDirection = a_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_RIGHT );
		vDirection.Normalise();
		pProjectile->m_uiAttackType = 0;

		if ( TRenderDX11::IsColorEqual( &vTarget.x, &TVector4::VEC_ZERO.x ) )
			pProjectile->Activate( a_pEntity, a_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), vDirection, 0x2F );
		else
			pProjectile->Fire( a_pEntity, a_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), vTarget, 0x2F );
	}
}

// Drops a pickup of the type at +0 from the entity at the speed +4, along: +8 0 the entity's right axis, 1 a random
// direction on the ground plane, 3 a random direction (other modes: no speed)
// NOTE: APickupData::CreatePickup returns TNULL on PC, so nothing is dropped
// $deBlob: FUNCTION 007ca6c0
void __stdcall IOCallbacks::OnSpawnObject( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	const TINT* pParams = TSTATICCAST( TINT, TSTATICCAST( AIOEntity::IOAction, a_pAction )->pParams );
	APickup*    pPickup = APickupData::CreatePickup( pParams[ 0 ] );
	TVector4    vVelocity( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1

	if ( pPickup == TNULL )
		return;

	TINT iMode = pParams[ 2 ];

	if ( iMode == 0 )
	{
		vVelocity = a_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_RIGHT );
		vVelocity.Normalise();
		vVelocity.Multiply( TREINTERPRETCAST( const TFLOAT*, pParams )[ 1 ] );
	}
	else if ( iMode == 1 )
	{
		TRandom& rRandom = ARandom::GetSingleton()->GetRandom();
		vVelocity.x      = rRandom.GetFloat2();
		vVelocity.z      = rRandom.GetFloat2();
		vVelocity.y      = 0.0f;
		vVelocity.NormaliseXZ();
		vVelocity.Multiply( TREINTERPRETCAST( const TFLOAT*, pParams )[ 1 ] );
	}
	else if ( iMode == 3 )
	{
		TRandom& rRandom = ARandom::GetSingleton()->GetRandom();
		vVelocity.x      = rRandom.GetFloat2();
		vVelocity.z      = rRandom.GetFloat2();
		vVelocity.y      = rRandom.GetFloat2();
		vVelocity.Normalise();
		vVelocity.Multiply( TREINTERPRETCAST( const TFLOAT*, pParams )[ 1 ] );
	}

	pPickup->SetPosition( a_pEntity->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );
	pPickup->SetMatrix( a_pEntity->GetMatrix() );
	pPickup->Drop( a_pEntity, vVelocity );
	pPickup->Show();
}

// The targets get the action as an input message
// $deBlob: FUNCTION 007ca7d0
void __stdcall IOCallbacks::OnTriggerTargets( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

	if ( pTargets == TNULL )
		return;

	for ( TINT i = 0; i < pTargets->iNumTargets; i++ )
	{
		ABaseEntity* pTarget = pTargets->pTargets[ i ].pEntity;

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail calls the target's HandleInputMessage even when the target wasn't found (TNULL)
		if ( pTarget == TNULL )
			continue;
#endif

		pTarget->CastTo<AIOEntity>()->HandleInputMessage( TSTATICCAST( TINT, a_pAction ), a_pEntity );
	}
}

// $deBlob: FUNCTION 007ca860
void __stdcall IOCallbacks::OnRotationStop( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	a_pEntity->StopRotations();
}

// $deBlob: FUNCTION 00785700
TBOOL AIOEntity::AreFlagsSet( TINT a_iCombination ) const
{
	TUINT32 uiFlags = m_uiFlags;

	switch ( a_iCombination )
	{
		case 0: return ( uiFlags & 1 ) != 0;
		case 1: return ( uiFlags & 2 ) != 0;
		case 2: return ( uiFlags & 4 ) != 0;
		case 3: return ( uiFlags & 8 ) != 0;
		case 4: return ( uiFlags & 1 ) && ( uiFlags & 2 );
		case 5: return ( uiFlags & 1 ) && ( uiFlags & 2 ) && ( uiFlags & 4 );
		case 6: return ( uiFlags & 1 ) && ( uiFlags & 2 ) && ( uiFlags & 8 );
		case 7: return ( uiFlags & 1 ) && ( uiFlags & 4 );
		case 8: return ( uiFlags & 1 ) && ( uiFlags & 4 ) && ( uiFlags & 8 );
		case 9: return ( uiFlags & 1 ) && ( uiFlags & 8 );
		case 10: return ( uiFlags & 2 ) && ( uiFlags & 4 );
		case 11: return ( uiFlags & 2 ) && ( uiFlags & 4 ) && ( uiFlags & 8 );
		case 12: return ( uiFlags & 2 ) && ( uiFlags & 8 );
		case 13: return ( uiFlags & 4 ) && ( uiFlags & 8 );
		case 14: return ( uiFlags & 1 ) && ( uiFlags & 2 ) && ( uiFlags & 4 ) && ( uiFlags & 8 );
		default: return TFALSE;
	}
}
