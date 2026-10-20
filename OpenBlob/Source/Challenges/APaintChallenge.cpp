#include "pch.h"
#include "APaintChallenge.h"
#include "AChallengeManager.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "Terrain2/ATerrain2.h"
#include "Terrain2/ATerrain2Interface.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"

#include <Platform/DX11/TWorldShader_DX11.h>
#include <Shaders/WorldShader/TWorldShader.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static TBOOL s_bProjections = TTRUE; // the targets get a projection (TWorldShaderHAL::ProjTexture)
static TBOOL s_bGlintUnk    = TTRUE;

// The first projection the targets use (TWorldShaderHAL::MAX_PROJ_TEXTURES_ALLOC - 2, downwards)
static TCONSTEXPR_VAR TINT s_iFirstProjection = 89;

static ATreeInstance* GetTree( void* a_pModelInstance2 ) { return TSTATICCAST( ATreeInstance, a_pModelInstance2 ); }

static TerrainTypes::CellInstance* GetInstance( const APaintChallenge::Target& a_rTarget )
{
	return TSTATICCAST( TerrainTypes::CellInstance, a_rTarget.m_pInstance );
}

static TUINT32 GetInstanceType( const APaintChallenge::Target& a_rTarget )
{
	return GetInstance( a_rTarget )->m_uiFlags & 7;
}

// Is a_pBuilding painted in the target's colour (-1: any colour but white and ink)?
static TBOOL IsPaintedAsTarget( const APaintChallenge::Target& a_rTarget, ABuildingInstance* a_pBuilding )
{
	TUINT16 uiColour = TUINT16( a_rTarget.m_iColour );

	return ( uiColour == 0xFFFF && a_pBuilding->m_uiCurrentColour != ABlobColourComponent::COLOUR_WHITE && a_pBuilding->m_uiCurrentColour != ABlobColourComponent::COLOUR_BLACK ) ||
	    a_pBuilding->m_uiCurrentColour == uiColour;
}

static ABlobChallengeStatusHud* GetStatusHud()
{
	return TSTATICCAST( ABlobChallengeStatusHud, AChallengeManager::GetSingleton()->GetHudElement() );
}

// Blob landed (rolling or state 6): the paint challenge is failed
static TBOOL HasBlobLanded()
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

	return pBlob != TNULL && ( pBlob->GetBlobState() == ABlobEntity::STATE_ROLL || pBlob->GetBlobState() == ABlobEntity::STATE_FLOAT );
}

// $deBlob: FUNCTION 0059c820
APaintChallenge::APaintChallenge()
    : AChallenge( Type_Paint )
{
	m_bPaintingStarted = TFALSE;
}

// $deBlob: FUNCTION 0059e1c0
APaintChallenge::~APaintChallenge()
{
}

// $deBlob: FUNCTION 0059e1d0
TBOOL APaintChallenge::Create( TypeData* a_pData )
{

	if ( !AChallenge::Create( a_pData ) )
		return TFALSE;

	for ( TUINT32 i = 0; i < a_pData->m_uiNumTargets; i++ )
	{
		Target& rTarget = a_pData->m_pTargets[ i ];

		if ( rTarget.m_iInstance == -1 )
		{
			rTarget.m_pInstance = TNULL;
			continue;
		}

		// The terrain instances become their instances
		TerrainTypes::CellInstance* pInstance = ATerrain2::GetInstance()->GetCellInstance( rTarget.m_iInstance );

		if ( pInstance == TNULL )
			continue;

		rTarget.m_pInstance = pInstance;

		if ( ( pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree )
		{
			GetTree( pInstance->m_pModelInstance2 )->CreateGlintMaterials();
		}
		else if ( m_pData->m_uiFlags & Data::Flags_LockBuildings )
		{
			TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 )->Lock();
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0059e2c0
void APaintChallenge::Start()
{
	m_bPaintingStarted = TFALSE;
	AJellyShader_DX11::GetSingleton()->FinishAll();

	TypeData*    pData       = TSTATICCAST( TypeData, m_pData );
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	TINT         iNumDone    = 0;
	TINT         iProjection = s_iFirstProjection;

	TASSERT( TWorldShader::GetSingleton() == TNULL || TWorldShader::GetSingleton()->IsA( &TGetClass( TWorldShaderHAL ) ) );

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		Target&                     rTarget   = pData->m_pTargets[ i ];
		TerrainTypes::CellInstance* pInstance = GetInstance( rTarget );
		TUINT32                     uiType    = pInstance->m_uiFlags & 7;

		if ( uiType == TerrainTypes::InstanceType_Building || uiType == TerrainTypes::InstanceType_Landmark )
		{
			ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 );

			if ( pInstance == TREINTERPRETCAST( TerrainTypes::CellInstance*, -1 ) || pBuilding == TNULL )
			{
				iNumDone++;
				rTarget.m_iDone = 1;
				continue;
			}

			pBuilding->m_iGlintIndex = -1;

			if ( uiType == TerrainTypes::InstanceType_Landmark )
			{
				TASSERT( TSTATICCAST( ALandmarkInstance, pBuilding )->IsABillBoard() );

				if ( pBuilding->m_uiFlags & ALandmarkInstance::Flags_Completed )
				{
					iNumDone++;
					rTarget.m_iDone = 1;
				}
			}
			else
			{
				pBuilding->SetTargetColour( TUINT8( rTarget.m_iColour ) );

				if ( m_pData->m_uiFlags & Data::Flags_LockBuildings )
					pBuilding->Unlock();

				TBOOL bDone = IsPaintedAsTarget( rTarget, pBuilding );

				if ( bDone )
					iNumDone++;

				rTarget.m_iDone = bDone;
			}

			if ( s_bProjections )
			{
				if ( rTarget.m_iDone == 0 )
					AGameStatus::SetBuildingProjection( pBuilding, iProjection, TTRUE );
				else
					pGameStatus->FreeProjection( iProjection );

				iProjection--;
			}
		}
		else if ( uiType == TerrainTypes::InstanceType_Tree )
		{
			if ( iProjection > 8 )
			{
				pGameStatus->FreeProjection( iProjection );
				iProjection--;
			}

			void* pTree = pInstance->m_pModelInstance2;

			if ( pInstance != TREINTERPRETCAST( TerrainTypes::CellInstance*, -1 ) && pTree != TNULL && !GetTree( pTree )->IsPainted() )
			{
				rTarget.m_iDone = 0;
				continue;
			}

			rTarget.m_iDone = 1;
			iNumDone++;
		}
	}

	GetStatusHud()->OnChallengeStart( this );
	AChallengeManager::GetSingleton()->SetCounter( iNumDone, pData->m_uiNumTargets );
	AChallenge::Start();
}

// $deBlob: FUNCTION 0059e550
void APaintChallenge::OnEnd()
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		TerrainTypes::CellInstance* pInstance = GetInstance( pData->m_pTargets[ i ] );
		TUINT32                     uiType    = pInstance->m_uiFlags & 7;

		if ( uiType == TerrainTypes::InstanceType_Building || uiType == TerrainTypes::InstanceType_Landmark )
		{
			ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 );

			if ( pInstance != TREINTERPRETCAST( TerrainTypes::CellInstance*, -1 ) && pBuilding != TNULL )
			{
				pBuilding->SetTargetColour( 0xFF );

				if ( s_bProjections )
					AGameStatus::RemoveBuildingProjection( pBuilding );
			}
		}
		else if ( uiType == TerrainTypes::InstanceType_Tree && s_bProjections )
		{
			GetTree( pInstance->m_pModelInstance2 )->m_iGlintIndex = -1;
		}
	}

	AGameStatus::GetSingleton()->ClearProjections();
}

// $deBlob: FUNCTION 0059e5d0
void APaintChallenge::OnPaintEvent( void* a_pBuilding )
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	if ( HasBlobLanded() && !( pData->m_uiFlags & Data::Flags_NoFail ) )
	{
		AChallengeManager::GetSingleton()->OnChallengeTimeOut( this );
		return;
	}

	ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, a_pBuilding );

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		Target&                     rTarget   = pData->m_pTargets[ i ];
		TerrainTypes::CellInstance* pInstance = GetInstance( rTarget );
		TUINT32                     uiType    = pInstance->m_uiFlags & 7;

		if ( ( uiType != TerrainTypes::InstanceType_Building && uiType != TerrainTypes::InstanceType_Landmark ) || pInstance->m_pModelInstance2 != a_pBuilding )
			continue;

		ABlobChallengeStatusHud* pHud = GetStatusHud();

		if ( uiType == TerrainTypes::InstanceType_Landmark )
		{
			TASSERT( TSTATICCAST( ALandmarkInstance, pBuilding )->IsABillBoard() );

			if ( pHud != TNULL && rTarget.m_iDone == 0 )
				pHud->UpdateColourCount( &rTarget, TFALSE );

			rTarget.m_iDone    = 1;
			m_bPaintingStarted = TTRUE;
		}
		else if ( IsPaintedAsTarget( rTarget, pBuilding ) )
		{
			if ( rTarget.m_iDone == 0 )
			{
				if ( pHud != TNULL )
					pHud->UpdateColourCount( &rTarget, TFALSE );

				rTarget.m_iDone = 1;
			}

			m_bPaintingStarted = TTRUE;
		}
		else
		{
			if ( pHud != TNULL && rTarget.m_iDone != 0 )
				pHud->UpdateColourCount( &rTarget, TTRUE );

			rTarget.m_iDone = 0;
		}

		if ( s_bProjections )
		{
			if ( pBuilding->m_iGlintIndex != -1 )
				AGameStatus::GetSingleton()->FreeProjection( pBuilding->m_iGlintIndex );

			AGameStatus::RemoveBuildingProjection( pBuilding );
		}

		// Projected again once its jelly is done (Update)
		if ( rTarget.m_iDone == 0 )
			pBuilding->m_uiFlags |= ABuildingInstance::Flags_Reproject;
	}

	TINT iNumDone = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		if ( pData->m_pTargets[ i ].m_iDone != 0 )
			iNumDone++;
	}

	AChallengeManager::GetSingleton()->SetCounter( iNumDone, pData->m_uiNumTargets );
}

// $deBlob: FUNCTION 0059e7d0
void APaintChallenge::OnPaintEvent2( void* a_pTree )
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	if ( HasBlobLanded() && !( pData->m_uiFlags & Data::Flags_NoFail ) )
	{
		AChallengeManager::GetSingleton()->OnChallengeTimeOut( this );
		return;
	}

	// A tree was painted (a_pTree: the ATreeInstance)
	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		Target&                     rTarget   = pData->m_pTargets[ i ];
		TerrainTypes::CellInstance* pInstance = GetInstance( rTarget );

		if ( ( pInstance->m_uiFlags & 7 ) != TerrainTypes::InstanceType_Tree || pInstance->m_pModelInstance2 != a_pTree )
			continue;

		TBOOL bPainted = TFALSE;
		TINT  iColour  = GetTree( a_pTree )->GetColour();

		if ( rTarget.m_iColour == -1 )
			bPainted = iColour != ABlobColourComponent::COLOUR_WHITE && iColour != ABlobColourComponent::COLOUR_BLACK;
		else
			bPainted = rTarget.m_iColour == iColour;

		ABlobChallengeStatusHud* pHud = GetStatusHud();

		if ( rTarget.m_iDone == 0 )
		{
			if ( bPainted )
			{
				rTarget.m_iDone    = 1;
				m_bPaintingStarted = TTRUE;

				if ( pHud != TNULL )
					pHud->UpdateColourCount( &rTarget, TFALSE );

				if ( s_bProjections )
				{
					AGameStatus::GetSingleton()->FreeProjection( GetTree( a_pTree )->m_iGlintIndex );
					GetTree( a_pTree )->m_iGlintIndex = -1;
				}
			}
		}
		else if ( !bPainted )
		{
			rTarget.m_iDone = 0;

			if ( pHud != TNULL )
				pHud->UpdateColourCount( &rTarget, TTRUE );
		}
	}

	TINT iNumDone = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		if ( pData->m_pTargets[ i ].m_iDone != 0 )
			iNumDone++;
	}

	AChallengeManager::GetSingleton()->SetCounter( iNumDone, pData->m_uiNumTargets );
}

// $deBlob: FUNCTION 0059e960
void APaintChallenge::Update( TFLOAT a_fDeltaTime )
{
	AChallenge::Update( a_fDeltaTime );

	if ( ( m_uiFlags & Flags_Completed ) || ( m_uiFlags & Flags_Failed ) )
		return;

	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	if ( m_bPaintingStarted == TTRUE && HasBlobLanded() && !( pData->m_uiFlags & Data::Flags_NoFail ) )
	{
		AChallengeManager::GetSingleton()->OnChallengeTimeOut( this );
		return;
	}

	ACamera* pCamera  = ACameraManager::GetSingleton()->GetCurrentCamera();
	TBOOL    bAllDone = TTRUE;

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		Target&                     rTarget   = pData->m_pTargets[ i ];
		TerrainTypes::CellInstance* pInstance = GetInstance( rTarget );

		if ( rTarget.m_iDone == 0 )
			bAllDone = TFALSE;

		if ( ( pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree )
		{
			void* pTree = pInstance->m_pModelInstance2;

			if ( pTree == TNULL )
				continue;

			TINT  iColour = ( rTarget.m_iColour != -1 ) ? rTarget.m_iColour : 0;
			TBOOL bUnk    = s_bGlintUnk != TFALSE;

			if ( pCamera == TNULL )
				continue;

			TVector4 vPosition;
			TSTATICCAST( AModelInstance2, pTree )->GetPosition( vPosition );

			TBOOL bVisible = ATerrain2Interface::IsInstanceInView( pInstance );

			const TVector4& vCamera  = pCamera->GetMatrix().GetTranslation();
			TFLOAT          fX       = vPosition.x - vCamera.x;
			TFLOAT          fZ       = vPosition.z - vCamera.z;
			TBOOL           bProject = s_bProjections && bVisible && fZ * fZ + fX * fX <= 40000.0f && rTarget.m_iDone == 0;

			if ( GetTree( pTree )->m_iGlintIndex == -1 )
			{
				TINT iProjection;

				if ( bProject && ( iProjection = AGameStatus::GetSingleton()->AllocateProjection() ) != -1 )
				{
					AGameStatus::SetTreeProjection( pInstance, iProjection, TTRUE );
				}
			}
			else if ( bProject )
			{
				AGameStatus::UpdateTreeGlint( GetTree( pTree ), a_fDeltaTime );
				AGameStatus::UpdateTreeGlintCamera( GetTree( pTree ), pCamera, iColour, bUnk );
			}
			else
			{
				AGameStatus::GetSingleton()->FreeProjection( GetTree( pTree )->m_iGlintIndex );
				GetTree( pTree )->m_iGlintIndex = -1;
			}
		}
		else
		{
			ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 );

			if ( pBuilding == TNULL )
				continue;

			TINT  iColour = ( rTarget.m_iColour != -1 ) ? rTarget.m_iColour : 0;
			TBOOL bUnk    = s_bGlintUnk != TFALSE;

			if ( pBuilding->m_iGlintIndex != -1 )
			{
				AGameStatus::UpdateGlint( pBuilding, a_fDeltaTime );
				AGameStatus::UpdateGlintCamera( pBuilding, pCamera, iColour, bUnk );
			}

			if ( ( pBuilding->m_uiFlags & ABuildingInstance::Flags_Reproject ) && !AJellyShader_DX11::GetSingleton()->IsPainting( pBuilding ) && rTarget.m_iDone == 0 )
			{
				pBuilding->m_uiFlags &= ~ABuildingInstance::Flags_Reproject;
				TINT iProjection = AGameStatus::GetSingleton()->AllocateProjection();

				if ( s_bProjections && iProjection != -1 )
					AGameStatus::SetBuildingProjection( pBuilding, iProjection, TFALSE );
			}
		}
	}

	if ( !bAllDone )
		return;

	AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );

	if ( m_pData->m_uiFlags & Data::Flags_LockBuildings )
	{
		for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
		{
			TerrainTypes::CellInstance* pInstance = GetInstance( pData->m_pTargets[ i ] );

			if ( ( pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Building )
				TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 )->SetCollisionLayers( TTRUE );
		}
	}
}

// $deBlob: FUNCTION 0059ec60
void APaintChallenge::Reset( TBOOL a_bUnk )
{
	AChallenge::Reset( a_bUnk );

	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		Target&                     rTarget   = pData->m_pTargets[ i ];
		TerrainTypes::CellInstance* pInstance = GetInstance( rTarget );
		TUINT32                     uiType    = pInstance->m_uiFlags & 7;

		rTarget.m_iDone = 0;

		if ( uiType == TerrainTypes::InstanceType_Building || uiType == TerrainTypes::InstanceType_Landmark )
		{
			ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 );
			pBuilding->m_uiFlags &= ~ABuildingInstance::Flags_Reproject;

			if ( ( m_pData->m_uiFlags & Data::Flags_LockBuildings ) && uiType == TerrainTypes::InstanceType_Building )
				pBuilding->Lock();

			if ( s_bProjections )
				AGameStatus::RemoveBuildingProjection( pBuilding );
		}
		else if ( uiType == TerrainTypes::InstanceType_Tree && s_bProjections )
		{
			GetTree( pInstance->m_pModelInstance2 )->m_iGlintIndex = -1;
		}
	}

	m_bPaintingStarted = TFALSE;
	AGameStatus::GetSingleton()->ClearProjections();
}

// $deBlob: FUNCTION 0059ed20
TINT APaintChallenge::GetScore()
{
	TypeData* pData     = TSTATICCAST( TypeData, m_pData );
	TUINT32   uiNumDone = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		if ( pData->m_pTargets[ i ].m_iDone != 0 )
			uiNumDone++;
	}

	return TINT( ( TFLOAT( uiNumDone ) / TFLOAT( pData->m_uiNumTargets ) ) * 100.0f );
}

// $deBlob: FUNCTION 0059ed90
TBOOL APaintChallenge::IsAlreadyCompleted()
{
	TypeData* pData     = TSTATICCAST( TypeData, m_pData );
	TUINT32   uiNumDone = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		Target&                     rTarget   = pData->m_pTargets[ i ];
		TerrainTypes::CellInstance* pInstance = GetInstance( rTarget );
		TUINT32                     uiType    = pInstance->m_uiFlags & 7;

		if ( uiType == TerrainTypes::InstanceType_Building || uiType == TerrainTypes::InstanceType_Landmark )
		{
			ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 );

			if ( uiType == TerrainTypes::InstanceType_Landmark )
			{
				TASSERT( TSTATICCAST( ALandmarkInstance, pBuilding )->IsABillBoard() );

				if ( pBuilding->m_uiFlags & ALandmarkInstance::Flags_Completed )
					uiNumDone++;
			}
			else
			{
				pBuilding->SetTargetColour( TUINT8( rTarget.m_iColour ) );

				if ( IsPaintedAsTarget( rTarget, pBuilding ) )
					uiNumDone++;
			}
		}
		else if ( uiType == TerrainTypes::InstanceType_Tree )
		{
			if ( GetTree( pInstance->m_pModelInstance2 )->IsPainted() )
				uiNumDone++;
		}
	}

	return uiNumDone == pData->m_uiNumTargets;
}

// $deBlob: FUNCTION 0059eeb0
void APaintChallenge::ToggleProjections()
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	if ( s_bProjections )
	{
		for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
		{
			TerrainTypes::CellInstance* pInstance = GetInstance( pData->m_pTargets[ i ] );
			TUINT32                     uiType    = pInstance->m_uiFlags & 7;

			if ( uiType == TerrainTypes::InstanceType_Building || uiType == TerrainTypes::InstanceType_Landmark )
				AGameStatus::RemoveBuildingProjection( TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 ) );
			else if ( uiType == TerrainTypes::InstanceType_Tree )
				GetTree( pInstance->m_pModelInstance2 )->m_iGlintIndex = -1;
		}

		s_bProjections = TFALSE;
		AGameStatus::GetSingleton()->ClearProjections();
		return;
	}

	TINT iProjection = s_iFirstProjection;
	TASSERT( TWorldShader::GetSingleton() == TNULL || TWorldShader::GetSingleton()->IsA( &TGetClass( TWorldShaderHAL ) ) );

	for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
	{
		Target&                     rTarget   = pData->m_pTargets[ i ];
		TerrainTypes::CellInstance* pInstance = GetInstance( rTarget );
		TUINT32                     uiType    = pInstance->m_uiFlags & 7;

		if ( uiType == TerrainTypes::InstanceType_Building || uiType == TerrainTypes::InstanceType_Landmark )
		{
			if ( rTarget.m_iDone == 0 )
			{
				AGameStatus::SetBuildingProjection( TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 ), iProjection, TTRUE );
				iProjection--;
			}
		}
		else if ( uiType == TerrainTypes::InstanceType_Tree && pInstance->m_pModelInstance2 != TNULL && iProjection > 8 )
		{
			AGameStatus::GetSingleton()->FreeProjection( iProjection );
			iProjection--;
		}
	}

	s_bProjections = TTRUE;
}
