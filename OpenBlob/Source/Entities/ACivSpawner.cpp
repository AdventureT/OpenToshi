#include "pch.h"
#include "APickupManager.h"
#include "ACivSpawner.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/TBoxData.h>
#include "AEntityPhysicsMover.h"
#include "AEntityFactory.h"
#include "AEntityParams.h"
#include "ARandom.h"
#include "ACiv.h"
#include "ACivWrapper.h"
#include "ASpawnedCivManager.h"
#include "APooledEntity.h"
#include "AWaypointData.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionObject.h"
#include "AEntityLoader.h"
#include "APlatformManager.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include <Platform/DX11/TTexture_DX11.h>
#include <Render/TModel.h>
#include "Buildings/ABuildingManager.h"
#include "Buildings/ABuildingInstance.h"
#include "Terrain2/ATerrain2.h"
#include "GameController/AGameStatus.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobColourComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TMaterial* ACivSpawner::ms_apSwapMaterials[ CIVSPAWNER_NOOFSWAPTEXTURES ];

TSTATICASSERT( sizeof( ACivSpawner ) == 0xE4 );

// The doorway textures of the swap materials
static const TCHAR* s_aszSwapTextures[ ACivSpawner::CIVSPAWNER_NOOFSWAPTEXTURES ] = {
	"prop_doorway_spawner2.tga",
	"prop_doorway_spawner3.tga",
	"prop_doorway_spawner4.tga",
};

// $deBlob: FUNCTION 0075f7a0
// $deBlob: FUNCTION 0075f7b0
// $deBlob: FUNCTION 0075f820
// $deBlob: FUNCTION 0075f880
// $deBlob: FUNCTION 0075f890
// $deBlob: FUNCTION 00540800
// $deBlob: FUNCTION 00859c70
TDEFINE_CLASS( ACivSpawner );

// $deBlob: FUNCTION 0075f8a0
ACivSpawner::ACivSpawner()
{
	m_iActiveCount            = 0;
	m_iNumTransformed         = 0;
	m_uiSpawnerFlags          = 0;
	m_pCollisionObject        = TNULL;
	m_pLinkedSplines          = TNULL;
	m_pBlock                  = TNULL;
	m_iBuildingInstance       = -1;
	m_iSecondBuildingInstance = -1;
	m_fSpawnDelay             = ARandom::GetSingleton()->GetRandom().GetFloat() * 0.40000004f + 0.2f;

	for ( TINT i = 0; i < CIVSPAWNER_MAXCIVCOUNT; i++ )
	{
		m_apCivs[ i ]       = TNULL;
		m_auiSlotFlags[ i ] = 0;
	}

	SetState( 0 );
}

// $deBlob: FUNCTION 0075f9b0
// $deBlob: FUNCTION 007613f0
ACivSpawner::~ACivSpawner()
{
	if ( m_pLinkedSplines )
	{
		::operator delete( m_pLinkedSplines->m_pSplines );
		::operator delete( m_pLinkedSplines );
	}
}

// $deBlob: FUNCTION 0075f9f0
void ACivSpawner::Tick( TFLOAT a_fDeltaTime )
{
	switch ( GetState() )
	{
		case 0:
			SetTicking( TFALSE );
			m_Flags |= FLAG_DontTick;
			break;

		case 1:
			if ( m_fSpawnDelay <= 0.0f )
			{
				m_fSpawnDelay = 0.1f;
				SetState( 2 );
				break;
			}

			m_fSpawnDelay -= a_fDeltaTime;
			break;

		case 2:
			if ( HasStateFinished() )
			{
				m_iNumTransformed = 0;
				SetState( 3 );
			}
			break;

		case 3:
			if ( m_iActiveCount > CIVSPAWNER_MAXCIVCOUNT - 1 )
			{
				SetState( 4 );
				break;
			}

			if ( m_fSpawnDelay <= 0.0f )
			{
				if ( SpawnCiv( m_iActiveCount, TTRUE ) )
				{
					m_iActiveCount++;
					m_fSpawnDelay = 0.1f;
				}
				else
				{
					m_fSpawnDelay = 0.2f;
				}

				break;
			}

			m_fSpawnDelay -= a_fDeltaTime;
			break;

		case 4:
			if ( HasStateFinished() )
				SetState( 0 );
			break;
	}

	AVisibleEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00760fd0
TBOOL ACivSpawner::SpawnCiv( TINT a_iPos, TBOOL a_bUseStateTable )
{
	TASSERT( !IsSpawned( a_iPos ) );

	// The state a civilian of a slot starts in (6, 8 or 7 by slot % 3), and the offset of a slot on the x and z axes
	static const TINT   s_aiStates[ 3 ] = { 6, 8, 7 };
	static const TFLOAT s_afScales[ 3 ] = { 6.0f, 15.0f, 12.0f };

	const Toshi::TMatrix44& mMatrix = GetMatrix();

	Toshi::TVector4 vPosition( mMatrix.m_f41, mMatrix.m_f42, mMatrix.m_f43, mMatrix.m_f44 );
	Toshi::TVector4 vFacing( mMatrix.m_f31, mMatrix.m_f32, mMatrix.m_f33, 1.0f );

	// The facing turns around the spawner (by the slot), the slot is at the distance of the slot along it
	vFacing.RotateY( ( 0.5f - TFLOAT( a_iPos ) / 7.0f ) * 1.5707964f + 3.1415927f );

	Toshi::TVector4 vOffset;
	vOffset.Multiply( vFacing, s_afScales[ a_iPos % 3 ] );

	vPosition.x += vOffset.x;
	vPosition.y += vOffset.y;
	vPosition.z += vOffset.z;

	if ( ( m_uiSpawnerFlags & 0x10 ) != 0 )
	{
		// Linked splines: the slot starts at its waypoint
		const SplineEntry& rSpline = m_pLinkedSplines->m_pSplines[ a_iPos ];
		vPosition                  = AWaypointData::GetSingleton()->GetSet( rSpline.m_iSetIndex ).GetNode( rSpline.m_iNodeIndex ).vPosition;
	}

	ACivWrapper* pWrapper = ASpawnedCivManager::GetSingleton()->AcquireWrapper( vPosition, vFacing, TREINTERPRETCAST( TINT, this ) );

	m_apCivs[ a_iPos ] = pWrapper;

	if ( pWrapper == TNULL )
		return TFALSE;

	TUINT8 uiSlotFlags = m_auiSlotFlags[ a_iPos ];

	if ( pWrapper->m_pPooledEntity == TNULL || pWrapper->m_pPooledEntity->m_pEntity == TNULL )
	{
		if ( ( uiSlotFlags & 0x1F ) != 0 )
			pWrapper->m_uiColour = uiSlotFlags & 0x1F;
	}
	else
	{
		ABaseEntity* pEntity = pWrapper->m_pPooledEntity->m_pEntity;
		ACiv*        pCiv    = TSTATICCAST( ACiv, pEntity );

		pCiv->SetTypeAndReset( 2 );

		if ( ( uiSlotFlags & 0x1F ) != 0 )
		{
			pCiv->Liberate( uiSlotFlags & 0x1F, TTRUE );
			pCiv->ResumeWalking();
		}
		else if ( a_bUseStateTable )
		{
			pEntity->SetState( s_aiStates[ a_iPos % 3 ] );
		}
		else
		{
			pEntity->SetState( 1 );
		}

		// The pose controller's byte at 0x2C is set (the name of its class is unknown), then the ACiv is active and visible
		TUINT8* pPoseController = TREINTERPRETCAST( TUINT8*, pEntity->GetPoseController() );
		pPoseController[ 0x2C ] = 1;

		pEntity->SetActive( TTRUE );
		pEntity->SetVisible( TTRUE );
	}

	m_auiSlotFlags[ a_iPos ] |= 0x20;

	return TTRUE;
}

// $deBlob: FUNCTION 007627a0
// The colour of a wrapper's civilian: its ACiv's (the pooled entity's), or the one the wrapper keeps without one
static TUINT8 GetWrapperColour( ACivWrapper* a_pWrapper )
{
	if ( a_pWrapper->m_pPooledEntity != TNULL )
		return TSTATICCAST( ACiv, a_pWrapper->m_pPooledEntity->m_pEntity )->GetColour();

	return a_pWrapper->m_uiColour;
}

// $deBlob: FUNCTION 007612a0
void ACivSpawner::ReleaseSlot( TINT a_iPos )
{
	TASSERT( IsSpawned( a_iPos ) );

	TUINT8 uiColour = GetWrapperColour( TREINTERPRETCAST( ACivWrapper*, m_apCivs[ a_iPos ] ) );
	AddSlotColour( a_iPos, uiColour );
	ASpawnedCivManager::GetSingleton()->ReleaseWrapper( TREINTERPRETCAST( ACivWrapper*, m_apCivs[ a_iPos ] ) );
	m_apCivs[ a_iPos ] = TNULL;
	m_auiSlotFlags[ a_iPos ] &= ~0x20;
}

// $deBlob: FUNCTION 0075fb00
void ACivSpawner::ProcessParams( const AEntityParams& a_rParams )
{
	AEntityParam::Collision** ppCollision = TNULL;

	if ( a_rParams.GetCollision( ppCollision ) )
	{
		CreatePhysics( m_pTypeData->m_pTRBData, *ppCollision, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );

		TPhysicsObjectContainer* pBody = m_pPhysicsData->m_pBody;

		if ( pBody )
			TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, ( pBody->m_uiCollisionFilterInfo & 0xFEF ) | 0x10 );
		else
			TPhysicsManager::GetSingleton()->SetCollisionLayer( TNULL, 0x10 );
	}
	else
	{
		// The building's box
		TBoxData oBox;
		oBox.SetHalfExtents( TVector4( 4.665f, 2.725f, 1.25f, 1.0f ) );
		TShapeBaseClass* pShape = &oBox;

		TMatrix44 matOffset;
		matOffset.Identity();
		TVector4 vOffset( -0.31f, -2.725f, -0.79f, 1.0f );
		TVector4 vTranslation;
		TMatrix44::TransformVector( vTranslation, matOffset, vOffset );
		matOffset.SetTranslation( vTranslation );

		CreatePhysics( m_pTypeData->m_pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}

	// The model renders its doorway with RenderDoorway (the render method it had is kept)
	TModelInstance* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();
	m_fnRender                     = pModelInstance->GetCustomRenderCb();
	m_pRenderUserData              = pModelInstance->GetCustomRenderCbUserData();
	pModelInstance->SetCustomRenderMethod( RenderDoorway, this );

	if ( ms_apSwapMaterials[ 0 ] == TNULL )
	{
		// The swap materials: copies of the model's material with the doorway textures
		ASkinShader* pSkinShader = ASkinShader::GetSingleton();

		T2ModelPtr pModel   = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance()->GetModel();
		TModelLOD& rLOD     = pModel->GetLOD( 0 );
		TMaterial* pOrigMat = rLOD.GetMeshes()[ 0 ]->GetMaterial();

		for ( TINT i = 0; i < CIVSPAWNER_NOOFSWAPTEXTURES; i++ )
		{
			ASkinMaterialHAL* pMaterial = ASkinMaterialHAL::Cast( pSkinShader->CreateMaterial( TNULL ) );
			pMaterial->Create( 0 );
			ms_apSwapMaterials[ i ] = pMaterial;

			pMaterial->SetTextureNum( pOrigMat->GetTextureNum() );

			for ( TUINT32 j = 0; j < pOrigMat->GetTextureNum(); j++ )
				pMaterial->SetTexture( j, pOrigMat->GetTexture( j ) );

			pMaterial->SetName( pOrigMat->GetName() );
			pMaterial->SetTexture( 0, TTextureManager::GetSingleton()->FindTexture( s_aszSwapTextures[ i ] ) );
		}
	}

	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );

	if ( rEntityData.GetWaypointSetTargetCount() > 0 )
	{
		// The linked splines: a waypoint set and its start node each
		m_pLinkedSplines                      = new LinkedSplines;
		m_pLinkedSplines->m_iNumLinkedSplines = rEntityData.GetWaypointSetTargetCount();
		m_pLinkedSplines->m_pSplines          = new SplineEntry[ m_pLinkedSplines->m_iNumLinkedSplines ];

		for ( TINT i = 0; i < m_pLinkedSplines->m_iNumLinkedSplines; i++ )
		{
			TINT iSetIndex                                = rEntityData.GetWaypointSetTarget( i, 0 );
			m_pLinkedSplines->m_pSplines[ i ].m_iSetIndex = iSetIndex;

			// NOTE: retail copies the second word whole (the node index and m_iUnk06)
			*TREINTERPRETCAST( TUINT32*, &m_pLinkedSplines->m_pSplines[ i ].m_iNodeIndex ) = rEntityData.GetWaypointSetTarget( i, 1 );

			APlatformManager::GetSingleton()->GetGroup( &AWaypointData::GetSingleton()->GetSet( iSetIndex ) );
		}

		// The spawner uses its linked splines unless the first one's start node has a link to follow
		const SplineEntry&   rFirst  = m_pLinkedSplines->m_pSplines[ 0 ];
		AWaypointData::Node& rNode   = AWaypointData::GetSingleton()->GetSet( rFirst.m_iSetIndex ).GetNode( rFirst.m_iNodeIndex );
		TBOOL                bFollow = TFALSE;

		for ( TINT i = 0; i < rNode.uiLinkCount; i++ )
		{
			if ( rNode.GetLink( i ).uiFlags & AWaypointData::Link::FLAG_Toggled )
			{
				bFollow = TTRUE;
				break;
			}
		}

		if ( !bFollow )
			m_uiSpawnerFlags |= 8;
	}

	// The linked cell instances: the door surround and an optional second one
	AEntityParams::IDList* pInstances = TNULL;

	if ( a_rParams.GetInstances( pInstances ) )
	{
		TASSERT( 0 != pInstances->m_iCount && pInstances->m_iCount <= 2 && "The CivSpawner is linked to the wrong number of instances (needs 1 and only 1 - the door surround)" );

		m_iBuildingInstance = TINT16( pInstances->m_pIDs[ 0 ] );

		if ( pInstances->m_iCount > 1 )
			m_iSecondBuildingInstance = TINT16( pInstances->m_pIDs[ 1 ] );
	}

	AVisibleEntity::ProcessParams( a_rParams );
}

// $deBlob: FUNCTION 007601a0
void ACivSpawner::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
			// The doorway takes the colour of the second instance's building (the ink when it's inked)
			if ( m_iSecondBuildingInstance >= 0 )
			{
				ABuildingInstance* pBuilding = ABuildingManager::GetSingleton()->GetBuildingInstance( m_iSecondBuildingInstance );

				if ( pBuilding != TNULL )
				{
					if ( TUINT8( pBuilding->m_uiFlags >> 2 ) & 1 )
						PaintDoorway( 0x10, TFALSE );
					else
						PaintDoorway( pBuilding->m_uiCurrentColour, TFALSE );
				}
			}

			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
			if ( a_pCaller != TNULL )
			{
				// Touched by a blob: paints the doorway in its colour (for a paint point), or touches the second instance

				if ( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) )
				{
					ABlobEntity* pBlob    = TSTATICCAST( ABlobEntity, a_pCaller );
					TUINT32      uiColour = pBlob->GetColourComponent()->GetCurrentColour();

					if ( uiColour & 0x10 )
						uiColour = 0x10;
					else if ( uiColour == 0 )
					{
						AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
						return;
					}

					if ( m_iSecondBuildingInstance >= 0 )
					{
						TerrainTypes::CellInstance* pInstance = ATerrain2::GetInstance()->GetCellInstance( m_iSecondBuildingInstance );

						if ( pInstance != TNULL )
							ABuildingManager::OnInstanceTouched( pInstance, pBlob, TREINTERPRETCAST( const TVector4*, a_rPacket.m_iData ) );
					}
					else
					{
						if ( PaintDoorway( uiColour, TFALSE ) )
							pBlob->RemovePaintPoints( 1, TFALSE );
					}
				}
			}

			break;
	}

	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
}
// $deBlob: FUNCTION 007602b0
void ACivSpawner::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	// The medium LOD spawns the civilians of the empty slots
	if ( ( a_iLOD & 0x400 ) != 0 )
	{
		for ( TINT i = 0; i < m_iActiveCount; i++ )
		{
			if ( !IsSpawned( i ) )
				SpawnCiv( i, TFALSE );
		}

		return;
	}

	if ( ( a_iLOD & 0x200 ) == 0 )
		return;

	if ( 0 != GetState() )
	{
		m_iActiveCount = 8;
		SetState( 0 );
	}

	if ( ( m_iActiveCount == 0 && 0 == GetState() ) || ( m_uiSpawnerFlags & 0x4 ) != 0 )
	{
		TASSERT( m_pBlock != TNULL && "The CivSpawner doesn't seem to be associated to any block?" );

		if ( !HasLinkedSplines() && m_pLinkedSplines != TNULL )
			return;

		if ( m_pBlock == TNULL )
			return;

		// The block must be a spawner block (type bits 0x10) that isn't dispersed (0x20)
		TUINT8 uiBlockFlags = TREINTERPRETCAST( TUINT8*, m_pBlock )[ 0x1F ];

		if ( ( uiBlockFlags & 0x10 ) == 0 || ( uiBlockFlags & 0x20 ) != 0 )
			return;

		if ( HasLinkedSplines() )
		{
			for ( TINT i = 0; i < m_iActiveCount; i++ )
			{
				if ( IsSpawned( i ) )
					ReleaseSlot( i );
			}
		}

		// The block's civilians are shared out to its spawners
		ABuildingManager::GetSingleton()->DisperseCivs( *m_pBlock, this );
	}
}

// $deBlob: FUNCTION 00760430
void ACivSpawner::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );

	for ( TINT i = 0; i < m_iActiveCount; i++ )
	{
		m_apCivs[ i ]       = TNULL;
		m_auiSlotFlags[ i ] = 0;
	}

	m_uiSpawnerFlags &= 8;
	m_iActiveCount = 0;

	TFLOAT fRandom = ARandom::GetSingleton()->GetRandom().GetFloat();
	m_uiSpawnerFlags &= ~3;
	m_fSpawnDelay = fRandom * 0.40000004f + 0.2f;

	SetState( 0 );

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
	pModelInstance->UseLocalAmbientLight( TFALSE );
}

// $deBlob: FUNCTION 00760510
void ACivSpawner::StartSpawning()
{
	if ( TUINT8( m_Flags >> 24 ) & 1 )
		return;

	TASSERT( 0 == GetState() && m_iActiveCount < CIVSPAWNER_MAXCIVCOUNT );

	m_Flags &= ~FLAG_DontTick;
	SetTicking( TTRUE );
	SetState( 1 );
}

// $deBlob: FUNCTION 00760580
TBOOL ACivSpawner::PaintDoorway( TUINT32 a_uiColour, TBOOL a_bOnlyIfUnpainted )
{
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	TUINT32          uiOldColour    = pModelInstance->GetLocalAmbientColour();

	if ( a_uiColour & 0x10 )
	{
		// The ink: back to the default colour, the building is unpainted
		if ( uiOldColour != AModelInstance2::ms_uiDefaultLocalAmbientColour )
		{
			pModelInstance->SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
			pModelInstance->UseLocalAmbientLight( TFALSE );
		}

		if ( m_iBuildingInstance >= 0 )
		{
			ABuildingInstance* pBuilding = ABuildingManager::GetSingleton()->GetBuildingInstance( m_iBuildingInstance );
			pBuilding->SetColour( a_uiColour, -1, TFALSE, TFALSE );

			if ( pBuilding->m_uiFlags & ABuildingInstance::Flags_Painted )
				AGameStatus::GetSingleton()->GetLevelData().OnBuildingUnpainted( pBuilding, 0, TTRUE, &GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), 0 );
		}

		return TFALSE;
	}

	if ( uiOldColour == ABlobColourComponent::ms_auiColours[ a_uiColour ] )
		return TFALSE;

	if ( a_bOnlyIfUnpainted && AModelInstance2::ms_uiDefaultLocalAmbientColour != uiOldColour )
		return TFALSE;

	pModelInstance->SetLocalAmbientColour( ABlobColourComponent::ms_auiColours[ a_uiColour ] );
	pModelInstance->UseLocalAmbientColour( TTRUE );
	pModelInstance->UseLocalAmbientLight( TTRUE );
	pModelInstance->SetAmbientColour( ABlobColourComponent::ms_auiColours[ a_uiColour ] );

	if ( m_iBuildingInstance >= 0 )
	{
		ABuildingInstance* pBuilding = ABuildingManager::GetSingleton()->GetBuildingInstance( m_iBuildingInstance );
		AGameStatus::GetSingleton()->GetLevelData().OnBuildingPainted( pBuilding, 0, &GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), 0, TFALSE );
		pBuilding->SetColour( a_uiColour, -1, TFALSE, TFALSE );
	}

	return TTRUE;
}
// $deBlob: FUNCTION 007606a0
void ACivSpawner::PickSwapTexture()
{
	if ( ( m_uiSpawnerFlags & 3 ) != 0 )
		return;

	TUINT8 uiTexNo = TUINT8( ARandom::GetSingleton()->GetRandom().GetInt( CIVSPAWNER_NOOFSWAPTEXTURES ) + 1 );
	SetTexNo( uiTexNo );
}

// $deBlob: FUNCTION 00760710
void ACivSpawner::Create()
{
	AVisibleEntity::Create();
	CreateCollisionObject();
}

// $deBlob: FUNCTION 00760720
void ACivSpawner::Destroy()
{
	if ( ms_apSwapMaterials[ 0 ] != TNULL )
	{
		for ( TINT i = 0; i < CIVSPAWNER_NOOFSWAPTEXTURES; i++ )
		{
			ASkinMaterialHAL::Cast( ms_apSwapMaterials[ i ] )->OnDestroy();
			delete ms_apSwapMaterials[ i ];
			ms_apSwapMaterials[ i ] = TNULL;
		}
	}

	if ( m_pCollisionObject != TNULL )
		delete m_pCollisionObject;

	m_pCollisionObject = TNULL;

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 00760780
void __stdcall ACivSpawner::RenderDoorway( TModelInstance* a_pModelInstance, void* a_pUserData )
{
	ACivSpawner* pSpawner = TSTATICCAST( ACivSpawner, a_pUserData );
	TINT         iTex     = pSpawner->m_uiSpawnerFlags & 3;
	TASSERT( ( iTex - 1 ) < ( CIVSPAWNER_NOOFSWAPTEXTURES ) );

	if ( iTex == 0 )
	{
		pSpawner->m_fnRender( a_pModelInstance, pSpawner->m_pRenderUserData );
		return;
	}

	TASSERT( 1 == a_pModelInstance->GetModel()->GetLODCount() );

	// The meshes render with the swap material, in the default local ambient colour
	T2ModelPtr pModel   = a_pModelInstance->GetModel();
	TModelLOD& rLOD     = pModel->GetLOD( 0 );
	TMaterial* pOrigMat = rLOD.GetMeshes()[ 0 ]->GetMaterial();

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		TASSERT( rLOD.GetMeshes()[ i ]->GetMaterial() == pOrigMat );
		rLOD.GetMeshes()[ i ]->SetMaterial( ms_apSwapMaterials[ iTex - 1 ] );
	}

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, pSpawner->GetModelComponent() )->GetModelInstance();
	TUINT32          uiOldColour    = pModelInstance->GetLocalAmbientColour();
	pModelInstance->SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
	pSpawner->m_fnRender( a_pModelInstance, pSpawner->m_pRenderUserData );
	pModelInstance->SetLocalAmbientColour( uiOldColour );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		rLOD.GetMeshes()[ i ]->SetMaterial( pOrigMat );
}

// $deBlob: FUNCTION 007609f0
void ACivSpawner::OnCivTransformed( ACiv* a_pCiv )
{
	// NOTE: a_pCiv isn't read
	m_iNumTransformed++;

	if ( m_iNumTransformed != 8 )
		return;

	m_uiSpawnerFlags |= 4;

	// The pickup 18 units in front of the doorway (the forward axis turned around)
	const TMatrix44& rMatrix   = GetMatrix();
	TVector4         vPosition = rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	TVector4         vForward  = rMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vForward.w                 = 1.0f;
	vForward.RotateY( TMath::PI );

	TVector4 vOffset;
	vOffset.Multiply( vForward, 18.0f );
	vPosition.x += vOffset.x;
	vPosition.y += vOffset.y;
	vPosition.z += vOffset.z;

	if ( !AGameStatus::GetSingleton()->GetLevelData().m_pLevelProps->m_sprintLevel )
		APickupManager::CreatePickup( 2, vPosition, -1.0f, 30.0f );

	// The civilians walk to their linked splines (an equal share of the civilians per spline)
	if ( !( m_uiSpawnerFlags & 8 ) && m_pLinkedSplines != TNULL )
	{
		TINT iCivsPerSpline = TMath::CeilToInt( TFLOAT( m_iActiveCount ) / TFLOAT( m_pLinkedSplines->m_iNumLinkedSplines ) );

		for ( TINT i = 0; i < m_iActiveCount; i++ )
		{
			TINT iSpline = i / iCivsPerSpline;
			TASSERT( IsSpawned( i ) );

			ACivWrapper* pWrapper = TREINTERPRETCAST( ACivWrapper*, m_apCivs[ i ] );
			ACiv*        pCiv     = ( pWrapper->m_pPooledEntity != TNULL ) ? TSTATICCAST( ACiv, pWrapper->m_pPooledEntity->m_pEntity ) : TNULL;
			TASSERT( !pCiv->IsAttachedToSpline(), "Civ shouldn't be attached to spline at this point" );

			if ( pCiv != TNULL && !pCiv->IsAttachedToSpline() )
			{
				const SplineEntry& rSpline = m_pLinkedSplines->m_pSplines[ iSpline ];
				pCiv->SetWaypointSet( rSpline.m_iSetIndex, *TREINTERPRETCAST( const TINT*, &rSpline.m_iNodeIndex ), &pCiv->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), TNULL );
			}
		}
	}
}

// $deBlob: FUNCTION 00760c10
void ACivSpawner::ReleaseTransformed( ACiv* a_pCiv )
{
	TASSERT( a_pCiv->IsTransformed() );
	TASSERT( m_iActiveCount > 0 );
	TASSERT( m_iNumTransformed > 0 );

	// The slot of the civilian
	TINT iCiv = 0;

	while ( iCiv < m_iActiveCount )
	{
		ACivWrapper* pWrapper = TREINTERPRETCAST( ACivWrapper*, m_apCivs[ iCiv ] );
		ABaseEntity* pEntity  = pWrapper->m_pPooledEntity != TNULL ? pWrapper->m_pPooledEntity->m_pEntity : TNULL;

		if ( pEntity == a_pCiv )
			break;

		iCiv++;
	}

	TASSERT( iCiv < m_iActiveCount );

	ReleaseSlot( iCiv );
	m_iActiveCount--;
	m_iNumTransformed--;

	for ( TINT i = iCiv; i < m_iActiveCount; i++ )
	{
		m_apCivs[ i ]       = m_apCivs[ i + 1 ];
		m_auiSlotFlags[ i ] = m_auiSlotFlags[ i + 1 ];
	}

	m_apCivs[ m_iActiveCount ]       = TNULL;
	m_auiSlotFlags[ m_iActiveCount ] = 0;
}

// $deBlob: FUNCTION 00760db0
void ACivSpawner::SetNumCivs( TINT a_iNumCivs, TBOOL a_bUseSplines )
{
	for ( TINT i = 0; i < m_iActiveCount; i++ )
	{
		if ( IsSpawned( i ) )
			ReleaseSlot( i );
	}

	if ( a_bUseSplines )
	{
		if ( !HasLinkedSplines() )
		{
			a_iNumCivs = 0;
		}
		else
		{
			TASSERT( m_pLinkedSplines->m_iNumLinkedSplines > 0 );
			TASSERT( m_pLinkedSplines->m_iNumLinkedSplines <= CIVSPAWNER_MAXCIVCOUNT );
			a_iNumCivs = m_pLinkedSplines->m_iNumLinkedSplines;
			m_uiSpawnerFlags |= 0x10;
		}
	}

	if ( m_iActiveCount < a_iNumCivs )
	{
		TASSERT( 0 == m_iActiveCount );

		// The new slots get a random colour
		for ( TINT i = 0; i < a_iNumCivs; i++ )
		{
			TUINT8 uiColour = TUINT8( ABlobColourComponent::GetRandomColour() );
			AddSlotColour( i, uiColour );
		}
	}
	else
	{
		for ( TINT i = a_iNumCivs; i < m_iActiveCount; i++ )
			m_auiSlotFlags[ i ] = 0;
	}

	m_uiSpawnerFlags |= 4;
	m_iActiveCount    = TINT8( a_iNumCivs );
	m_iNumTransformed = TINT8( a_iNumCivs );
}
// $deBlob: FUNCTION 00760f70
void ACivSpawner::Despawn( TBOOL a_bPlayEffect )
{
	AVisibleEntity::Despawn( a_bPlayEffect );

	for ( TINT i = 0; i < m_iActiveCount; i++ )
	{
#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail releases every slot, but a slot is empty when the spawn found no free wrapper
		if ( m_apCivs[ i ] != TNULL )
			ASpawnedCivManager::GetSingleton()->ReleaseWrapper( TREINTERPRETCAST( ACivWrapper*, m_apCivs[ i ] ) );
#else
		ASpawnedCivManager::GetSingleton()->ReleaseWrapper( TREINTERPRETCAST( ACivWrapper*, m_apCivs[ i ] ) );
#endif

		m_apCivs[ i ] = TNULL;
	}
}

// $deBlob: FUNCTION 00760fc0
const TCHAR* ACivSpawner::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}

// $deBlob: FUNCTION 00761360
void ACivSpawner::CreateCollisionObject()
{
	if ( GetCollisionObject() == TNULL )
	{
		m_pCollisionObject = new ACollisionObject( this, 1, 0xFF );
		GetCollisionObject()->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
		m_pCollisionObject->SetFlags( ACollisionObject::FLAG_Created );
	}

	// NOTE: retail writes the low byte only
	*TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType ) = 0x20;
}
