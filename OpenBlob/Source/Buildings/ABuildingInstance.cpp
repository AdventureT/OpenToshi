#include "pch.h"
#include "Entities/ABlobEntity/ABlobSoundComponent.h"
#include "ABuildingInstance.h"
#include "ABuildingManager.h"
#include "Animation/AModel2.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "GameController/AGameStatus.h"
#include "Terrain2/ATerrain2.h"
#include "Challenges/AChallengeManager.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "Buildings/ALandmarkInstance.h"
#include "Entities/ACivSpawner.h"
#include "Entities/AEntityIOPacket.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

// Note: Should be the last include!
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TFLOAT ABuildingInstance::s_cfFadeTime      = 3.0f;
const TFLOAT ABuildingInstance::s_cfPaintFadeTime = 2.5f;

static TCONSTEXPR_VAR TFLOAT s_cfPulseFrequency = 12.0f;
static TCONSTEXPR_VAR TFLOAT s_cfPulseAmplitude = 0.1f;

// The type of an ABuildingManager PaintableBlock (FlagMask_Type of its m_uiFlags at 0x1F) NOTE: PaintableBlock isn't
// ported
static TUINT8 GetBlockType( void* a_pBlock )
{
	return TREINTERPRETCAST( TUINT8*, a_pBlock )[ 0x1F ] & 0xF;
}

// $deBlob: FUNCTION 00576b50
void ABuildingInstance::OnCreated( TerrainTypes::CellInstance* a_pInstance )
{
	ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, a_pInstance->m_pModelInstance2 );

	if ( pBuilding != TNULL )
	{
		pBuilding->m_pTerrainInstance = a_pInstance;
		ABuildingManager::GetSingleton()->AddBuilding( pBuilding );
	}
	else
	{
		a_pInstance->m_uiColour = AModelInstance2::ms_uiDefaultLocalAmbientColour;
	}
}

// $deBlob: FUNCTION 00576b80
void ABuildingInstance::OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const TVector4* a_pPosition )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Building );

	ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, a_pInstance->m_pModelInstance2 );

	if ( !pBuilding->IsAPaintableBuilding() )
		return;

	if ( !a_pBlob->IsA( &TGetClass( ABlobEntity ) ) || !pBuilding->CanBePaintedBy( a_pBlob ) )
		return;

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
	{
		ABuildingManager::GetSingleton()->AddActiveInstance( pBuilding );
		pBuilding->m_pPendingBlob     = a_pBlob;
		pBuilding->m_vPendingPosition = *a_pPosition;
		return;
	}

	// Multiplayer: the biggest Blob touching it wins
	TINT iSize = a_pBlob->GetSize();

	if ( pBuilding->m_iPendingPriority < iSize )
	{
		pBuilding->m_iPendingPriority = TINT8( iSize );
		ABuildingManager::GetSingleton()->AddActiveInstance( pBuilding );
		pBuilding->m_pPendingBlob     = a_pBlob;
		pBuilding->m_fPendingTimer    = 0.65f;
		pBuilding->m_vPendingPosition = *a_pPosition;
		return;
	}

	if ( iSize == pBuilding->m_iPendingPriority )
	{
		TUINT32 uiColour = a_pBlob->GetPaintColour();

		if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
			uiColour = ABlobColourComponent::COLOUR_BLACK;

		if ( pBuilding->m_uiCurrentColour == uiColour )
		{
			pBuilding->m_fPendingTimer = 0.65f;
			ABuildingManager::GetSingleton()->AddActiveInstance( pBuilding );
		}
	}
}

// $deBlob: FUNCTION 00576cd0
ABuildingInstance::ABuildingInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints )
    : AModelInstance2( a_pModel, a_bCreateSoundBreakpoints, TFALSE )
    , m_oNode( this )
{
	m_uiFlags              = 0;
	m_iMultiplayerOwner    = -1;
	m_uiTargetColour       = 0xFF;
	m_uiCurrentColour      = ABlobColourComponent::COLOUR_WHITE;
	m_uiCollisionLayerMask = 0;
	m_pTerrainInstance     = TNULL;
	m_fInktTimer           = 0.0f;
	m_fPaintFadeTimer      = 0.0f;
	m_iGlintIndex          = -1;
	UseLocalAmbientColour( TTRUE );
	m_fUnkBC = 1.0f;
	m_fUnkC0 = 1.0f;

	m_iTexScaleIndex = AJellyShader_DX11::GetSingleton()->AllocTexScale();

	m_uiUnkC6          = 0xFF;
	m_fPendingTimer    = -1.0f;
	m_iPendingPriority = -1;
	m_pPendingBlob     = TNULL;
	m_vPendingPosition = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_apBlocks[ 0 ]    = TNULL;
	m_apBlocks[ 1 ]    = TNULL;
}

// $deBlob: FUNCTION 00576e30
// $deBlob: FUNCTION 005783d0
ABuildingInstance::~ABuildingInstance()
{
	if ( m_oNode.IsLinked() )
		m_oNode.Remove();

	ABuildingManager::GetSingleton()->RemoveBuilding( this );
}

// $deBlob: FUNCTION 00576ee0
void ABuildingInstance::OnTable97b958( TerrainTypes::CellInstance* a_pInstance )
{
	if ( a_pInstance->m_pModelInstance2 == TNULL )
		a_pInstance->m_uiColour = AModelInstance2::ms_uiDefaultLocalAmbientColour;
}

// $deBlob: FUNCTION 00576f00
void ABuildingInstance::OnTable97b978( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Building );

	ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, a_pInstance->m_pModelInstance2 );

	if ( pBuilding == TNULL )
	{
		// A building without a streamed model: only its colour
		if ( a_pInstance->m_uiColour == AModelInstance2::ms_uiDefaultLocalAmbientColour )
			a_pInstance->m_uiColour = ABlobColourComponent::ms_auiColours[ ABlobColourComponent::GetRandomColour() ];

		return;
	}

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
	{
		TUINT32 uiColour = ABlobColourComponent::GetRandomColour();
		AGameStatus::GetSingleton()->GetLevelData().OnBuildingPainted( pBuilding, 0, TREINTERPRETCAST( TVector4*, &a_pInstance->m_pMatrix->m_f41 ), 0, TTRUE );
		pBuilding->SetColour( uiColour, -1, TFALSE, TFALSE );
		return;
	}

	TINT8 iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob );

	// NOTE: BlobScoringProperties isn't ported: 0x0C the score of a building, 0x04 the factor of a repainted one
	void* pScoring = AGameStatus::GetBlobScoringProperties();
	TINT  iScore   = TREINTERPRETCAST( TINT*, pScoring )[ 3 ];

	if ( pBuilding->m_uiFlags & Flags_Painted )
		iScore = TINT( TFLOAT( iScore ) * TREINTERPRETCAST( TFLOAT*, pScoring )[ 1 ] );

	AGameStatus::GetSingleton()->AddBonusScore( iPlayer, iScore, *a_pInstance->m_ppCentre );

	TUINT32 uiColour = a_pBlob->GetPaintColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	pBuilding->SetColour( uiColour, -1, TFALSE, TFALSE );
}

// $deBlob: FUNCTION 00577020
void ABuildingInstance::Paint( ABlobEntity* a_pBlob, const TVector4* a_pPosition )
{
	if ( !CanBePaintedBy( a_pBlob ) )
		return;

	TINT                  iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();
	ABlobColourComponent* pColour   = a_pBlob->GetColourComponent();
	TUINT32               uiColour  = pColour->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;

	TUINT32 uiARGB = pColour->GetFadeColour();

	if ( ( a_pBlob->GetBlobFlags() >> 21 ) & 1 )
	{
		// The colour of the terrain instance in the current paint challenge
		TINT iChallengeColour = AChallengeManager::GetSingleton()->GetPaintChallengeIndex( TINT( TREINTERPRETCAST( TUINTPTR, m_pTerrainInstance ) ) );

		if ( iChallengeColour != -1 && iChallengeColour != 0 )
		{
			uiColour = TUINT32( iChallengeColour );
			uiARGB   = ABlobColourComponent::ms_auiColours[ iChallengeColour ];
		}
	}

	if ( iGameType == 2 )
	{
		if ( uiColour == ABlobColourComponent::COLOUR_BLACK )
			return;

		if ( !AGameStatus::GetSingleton()->IsNextPaintableBuilding( this ) )
			return;
	}
	else if ( iGameType == 3 && uiColour != ABlobColourComponent::COLOUR_BLACK )
	{
		uiColour = pColour->GetUnk20();
		uiARGB   = ABlobColourComponent::ms_auiColours[ uiColour ];
	}

	APlayerManager* pPM     = APlayerManager::GetSingleton();
	TINT8           iPlayer = pPM->GetPlayerIndex( a_pBlob );

	if ( uiColour == m_uiCurrentColour )
	{
		// Already in this colour: repaints the jelly of a painted building in a multiplayer game
		if ( m_uiUnkC6 != 0xFF )
			return;

		if ( TINT( pPM->GetBlobDataOf( a_pBlob )->m_uiUnk58 ) < 1 )
			return;

		if ( ( m_pTerrainInstance->m_uiFlags & 0x110 ) != 0x110 )
			return;

		AChallengeManager::GetSingleton()->OnPaintEvent( this );
		SetPaintColour( uiARGB, iPlayer, TFALSE, -1 );
	}
	else
	{
		m_fPaintFadeTimer = s_cfPaintFadeTime;
		m_uiFlags         = ( m_uiFlags & ~Flags_FadeDone ) | Flags_Update;

		// BlobScoringProperties 0x0C: the score of a building, 0x10: the one of the flag 0x40000
		void* pScoring = AGameStatus::GetBlobScoringProperties();
		TINT  iScore   = ( m_pTerrainInstance->m_uiFlags & 0x40000 ) ? TREINTERPRETCAST( TINT*, pScoring )[ 4 ] : TREINTERPRETCAST( TINT*, pScoring )[ 3 ];

		// In a multiplayer game (the MP data is only loaded then) a building with a multiplayer score scores that instead
		if ( iGameType >= 0 && m_pTerrainInstance != TNULL && ATerrain2::GetInstance()->GetMPInstanceData( m_pTerrainInstance )->m_iUnk0 >= 0 )
			iScore = Get_MP_Score();

		TVector4 vPosition;
		GetPosition( vPosition );

		if ( m_iGlintIndex >= 0 && ( uiColour != ABlobColourComponent::COLOUR_BLACK || !( m_uiFlags & Flags_Inked ) ) )
		{
			// The paint challenge projection goes before the jelly copies the building's materials
			if ( m_iGlintIndex == 90 )
			{
				AGameStatus::RemoveBuildingProjection( this );
			}
			else if ( m_uiFlags & Flags_BossPlatform )
			{
				TINT iProj = AGameStatus::RemoveBuildingProjection( this );

				if ( iProj >= 0 )
					AGameStatus::GetSingleton()->FreeProjection( iProj );
			}
		}

		if ( uiColour != ABlobColourComponent::COLOUR_BLACK )
		{
			// Painted in a colour: the score chain of the player
			// The score chain of the player: a blob in state 1 or 2 (ABlobEntity::m_eState) starts or continues a combo
			if ( TUINT32( a_pBlob->GetBlobState() - 1 ) < 2 )
			{
				if ( AGameStatus::GetSingleton()->GetLevelData().GetComboState( iPlayer ) == 1 )
					AGameStatus::GetSingleton()->GetLevelData().IncrementComboCount( iPlayer );
				else
					AGameStatus::GetSingleton()->GetLevelData().StartCombo( iPlayer, 1 );
			}
			else
			{
				AGameStatus::GetSingleton()->GetLevelData().EndCombo( iPlayer );
			}
			AGameStatus::GetSingleton()->GetLevelData().OnBuildingPainted( this, iPlayer, a_pPosition, 0, TFALSE );

			if ( 0 <= iGameType && m_iMultiplayerOwner != iPlayer && m_iMultiplayerOwner != -1 )
			{
				// Taken from another player
				TASSERT( m_iMultiplayerOwner >= 0 && m_iMultiplayerOwner < pPM->GetNumPlayers() );
				AGameStatus::GetSingleton()->GetLevelData().AddScore( m_iMultiplayerOwner, -iScore, -1 );
				AGameStatus::GetSingleton()->GetLevelData().OnBuildingUnpainted( this, m_iMultiplayerOwner, TFALSE, &TVector4::VEC_ZERO, 0 );
			}

			m_iMultiplayerOwner = iPlayer;
			m_uiFlags           = ( m_uiFlags & ~Flags_Inked ) | Flags_Painted | Flags_PaintedOnce;

			if ( iGameType >= 0 || ( !( m_pTerrainInstance->m_uiFlags & 0x40000 ) && !( uiColour & ABlobColourComponent::COLOUR_BLACK ) ) )
				a_pBlob->RemovePaintPoints( 1, TFALSE );

			m_uiCurrentColour = TUINT8( uiColour );

			for ( TINT i = 0; i < MAXBLOCKS; i++ )
			{
				if ( m_apBlocks[ i ] != TNULL )
				{
					ABuildingManager::GetSingleton()->OnBlockBuildingPainted( this, iPlayer, *TSTATICCAST( PaintableBlock, m_apBlocks[ i ] ) );

					if ( GetBlockType( m_apBlocks[ i ] ) == 7 )
						ABuildingManager::GetSingleton()->RemoveJellyInstance( this );
				}
			}

			AGameStatus::GetSingleton()->OnBuildingPaintChanged( this );
			AChallengeManager::GetSingleton()->OnPaintEvent( this );
			SetPaintColour( uiARGB, iPlayer, TFALSE, -1 );
			AMusicManager::GetSingleton()->AddPaintSyncSound( uiColour, a_pBlob, &vPosition );
			ABuildingManager::TriggerLinks( m_pTerrainInstance, AEntityIOPacket::MESSAGE_TRIGGER );
		}
		else
		{
			// Inked
			if ( m_uiCurrentColour != ABlobColourComponent::COLOUR_WHITE )
			{
				if ( m_iMultiplayerOwner == -1 || m_iMultiplayerOwner == iPlayer )
				{
					AGameStatus::GetSingleton()->GetLevelData().OnBuildingUnpainted( this, iPlayer, TTRUE, a_pPosition, 0 );
				}
				else
				{
					AGameStatus::GetSingleton()->GetLevelData().OnBuildingUnpainted( this, m_iMultiplayerOwner, TFALSE, &TVector4::VEC_ZERO, 0 );
					AGameStatus::GetSingleton()->GetLevelData().AddScore( m_iMultiplayerOwner, -iScore, -1 );
				}

				// The score chain of the player: the combo of an inked building (state 4)
				if ( AGameStatus::GetSingleton()->GetLevelData().GetComboState( iPlayer ) == 4 )
					AGameStatus::GetSingleton()->GetLevelData().IncrementComboCount( iPlayer );
				else
					AGameStatus::GetSingleton()->GetLevelData().StartCombo( iPlayer, 4 );

				m_uiCurrentColour   = ABlobColourComponent::COLOUR_WHITE;
				m_iMultiplayerOwner = -1;
			}

			if ( m_uiFlags & Flags_Inked )
				return;

			AGameStatus::GetSingleton()->OnBuildingPaintChanged( this );
			AChallengeManager::GetSingleton()->OnPaintEvent( this );
			SetPaintColour( uiARGB, iPlayer, TFALSE, -1 );
			AMusicManager::GetSingleton()->AddPaintSyncSound( ABlobColourComponent::COLOUR_BLACK, a_pBlob, &vPosition );
			m_uiFlags    = ( m_uiFlags & ~Flags_Painted ) | Flags_Inked;
			m_fInktTimer = s_cfFadeTime;
			NotifyCivSpawners();
		}
	}

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x29 ), a_pBlob->GetBodyPosition(), 0 );
}

// $deBlob: FUNCTION 00577510
TINT ABuildingInstance::Get_MP_Score() const
{
	if ( m_pTerrainInstance != TNULL )
		return ATerrain2::GetInstance()->GetMPInstanceData( m_pTerrainInstance )->m_iUnk0;

	return -1;
}

// $deBlob: FUNCTION 00577530
TINT ABuildingInstance::Get_MP_BuildingRank() const
{
	if ( m_pTerrainInstance != TNULL )
		return ATerrain2::GetInstance()->GetMPInstanceData( m_pTerrainInstance )->m_iBuildingRank;

	return 0;
}

// $deBlob: FUNCTION 005774c0
TBOOL ABuildingInstance::IsAPaintableBuilding() const
{
	TUINT32 uiFlags = m_pTerrainInstance->m_uiFlags;

	if ( ( uiFlags & 7 ) != TerrainTypes::InstanceType_Building )
		return TFALSE;

	return !( uiFlags & 0x8000 ) || m_uiCurrentColour == ABlobColourComponent::COLOUR_BLACK || m_uiCurrentColour == ABlobColourComponent::COLOUR_WHITE;
}

// $deBlob: FUNCTION 00577550
TBOOL ABuildingInstance::CanBePaintedBy( ABlobEntity* a_pBlob )
{
	TASSERT( IsAPaintableBuilding() );

	if ( a_pBlob->GetPaintColour() == ABlobColourComponent::COLOUR_WHITE )
		return TFALSE;

	if ( !( m_pTerrainInstance->m_uiFlags & 0x400 ) )
		return TFALSE;

	if ( m_uiFlags & Flags_NotPaintable )
		return TFALSE;

	if ( m_uiFlags & Flags_Locked )
	{
		if ( a_pBlob->GetSoundComponent() != TNULL )
			a_pBlob->GetSoundComponent()->PlayLockedBuilding( a_pBlob );

		return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005775f0
void ABuildingInstance::SetPaintColour( TUINT32 a_uiColour, TINT a_iPlayer, TBOOL a_bInstant, TINT a_iUnk )
{
	TUINT32 uiFlags = m_pTerrainInstance->m_uiFlags;

	if ( ( uiFlags & 0x10 ) == 0 || ( uiFlags & 0x100 ) == 0 )
	{
		SetLocalAmbientColour( a_uiColour );
		return;
	}

	if ( ( uiFlags >> 24 ) & 1 )
	{
		TINT eSolid = AJellyShader_DX11::Solid_Drip;

		if ( a_iPlayer < 0 || APlayerManager::GetSingleton()->GetNumPlayers() < 2 )
		{
			if ( m_pTerrainInstance->m_uiFlags & 0x100000 )
				eSolid = AJellyShader_DX11::Solid_FastDrip;
		}
		else
		{
			eSolid = AJellyShader_DX11::Solid_MPDrip;
		}

		AJellyShader_DX11::GetSingleton()->StartPaint( this, GetLocalAmbientColour(), a_uiColour, eSolid, a_iPlayer, a_bInstant ? 2.0f : 1.0f );
	}
}

// $deBlob: FUNCTION 00577690
TINT ABuildingInstance::Reset()
{
	SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
	m_uiCurrentColour = ABlobColourComponent::COLOUR_WHITE;
	SetCollisionLayers( TTRUE );
	m_uiFlags           = 0;
	m_iMultiplayerOwner = -1;
	m_iPendingPriority  = -1;
	m_fInktTimer        = 0.0f;
	m_fPaintFadeTimer   = 0.0f;
	m_fUnkBC            = 1.0f;
	m_fUnkC0            = 1.0f;
	m_uiUnkC6           = 0xFF;
	m_fPendingTimer     = -1.0f;
	m_pPendingBlob      = TNULL;
	m_iGlintIndex       = -1;
	m_vPendingPosition  = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	return -1;
}

// $deBlob: FUNCTION 00577720
void ABuildingInstance::UpdateImp( TFLOAT a_fDeltaTime )
{
	if ( m_uiFlags & Flags_Pulse )
	{
		m_uiFlags |= Flags_Update;
		m_fPulseTimer += a_fDeltaTime;
		TMath::Clip( m_fPulseTimer, -10000.0f, 10000.0f );

		if ( 0.0f < m_fPulseTimer )
			SetScale( TFLOAT( sin( double( m_fPulseTimer * s_cfPulseFrequency ) ) ) * s_cfPulseAmplitude + ( 1.0f - s_cfPulseAmplitude ) );
	}

	if ( m_uiFlags & Flags_ScaleBack )
	{
		TASSERT( !( m_uiFlags & Flags_Pulse ) );

		TVector4 vScale = GetScale();
		TVector4 vOne( 1.0f, 1.0f, 1.0f, 1.0f );

		TFLOAT fT = a_fDeltaTime * 3.0f;
		TMath::Clip( fT, 0.0f, 1.0f );
		vScale = TMath::LERPVector4Clip( vScale, vOne, fT );

		if ( 0.01f <= TVector4::DistanceSq( vOne, vScale ) )
		{
			SetScale( vScale );
			m_uiFlags |= Flags_Update;
		}
		else
		{
			SetScale( vOne );
			m_uiFlags &= ~Flags_ScaleBack;
		}
	}

	if ( m_pPendingBlob != TNULL )
	{
		Paint( m_pPendingBlob, &m_vPendingPosition );
		m_pPendingBlob = TNULL;
	}

	if ( 0.0f < m_fPendingTimer )
	{
		m_fPendingTimer -= a_fDeltaTime;
		m_uiFlags |= Flags_Update;

		if ( m_fPendingTimer < 0.0f )
		{
			m_fPendingTimer    = -1.0f;
			m_iPendingPriority = -1;
		}
	}

	if ( m_uiFlags & Flags_Inked )
	{
		if ( !( ( m_pTerrainInstance->m_uiFlags >> 4 ) & 1 ) || !AJellyShader_DX11::GetSingleton()->IsPainting( this ) )
		{
			m_fInktTimer -= a_fDeltaTime;

			if ( 0.0f < m_fInktTimer )
			{
				if ( m_fInktTimer < 2.0f )
				{
					TFLOAT fT = m_fInktTimer * 0.5f;
					TASSERT( 0.0f <= m_fInktTimer && m_fInktTimer <= s_cfFadeTime );
					SetLocalAmbientColour( LerpColour( AModelInstance2::ms_uiDefaultLocalAmbientColour, ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_BLACK ], fT ) );
				}
			}
			else
			{
				SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
				m_uiFlags &= ~Flags_Inked;
				m_uiCurrentColour   = ABlobColourComponent::COLOUR_WHITE;
				m_iMultiplayerOwner = -1;
			}
		}

		m_uiFlags |= Flags_Update;
	}

	// The paint fades to the dark colour (in a multiplayer game type 2 and on flag 0x8000)
	TBOOL bFade = AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 2 || ( m_pTerrainInstance->m_uiFlags & 0x8000 );

	if ( m_pTerrainInstance != TNULL && bFade && !( m_uiFlags & Flags_FadeDone ) &&
	     m_uiCurrentColour != ABlobColourComponent::COLOUR_BLACK && m_uiCurrentColour != ABlobColourComponent::COLOUR_WHITE )
	{
		m_uiFlags |= Flags_Update;

		if ( !AJellyShader_DX11::GetSingleton()->IsPainting( this ) )
		{
			m_fPaintFadeTimer -= a_fDeltaTime;

			if ( m_fPaintFadeTimer <= 0.0f )
			{
				m_fPaintFadeTimer = 0.0f;
				m_uiFlags         = ( m_uiFlags & ~Flags_Update ) | Flags_FadeDone;
			}

			SetLocalAmbientColour( LerpColour( ABlobColourComponent::ms_auiColours[ m_uiCurrentColour ], ABlobColourComponent::ms_auiDarkColours[ m_uiCurrentColour ], 1.0f - m_fPaintFadeTimer / s_cfPaintFadeTime ) );
		}
	}
}

// $deBlob: FUNCTION 00577ae0
void ABuildingInstance::StartPulse( TFLOAT a_fDelay )
{
	TASSERT( a_fDelay > -10000.0f );
	TASSERT( GetScale().x == 1.0f );
	TASSERT( GetScale().y == 1.0f );
	TASSERT( GetScale().z == 1.0f );

	ABuildingManager::GetSingleton()->AddActiveInstance( this );
	m_uiFlags     = ( m_uiFlags & ~Flags_ScaleBack ) | Flags_Pulse;
	m_fPulseTimer = -a_fDelay;
}

// $deBlob: FUNCTION 00577c20
void ABuildingInstance::StartScaleBack()
{
	ABuildingManager::GetSingleton()->AddActiveInstance( this );
	m_uiFlags     = ( m_uiFlags & ~Flags_Pulse ) | Flags_ScaleBack;
	m_fPulseTimer = 0.0f;
}

// $deBlob: FUNCTION 00577c50
void ABuildingInstance::SetColour( TUINT32 a_uiColour, TINT a_iPlayer, TBOOL a_bJelly, TBOOL a_bUnk2 )
{
	TUINT8 uiOldColour = m_uiCurrentColour;

	if ( uiOldColour == a_uiColour )
		return;

	m_uiFlags &= ~Flags_FadeDone;
	m_fPaintFadeTimer = s_cfPaintFadeTime;

	TUINT32 uiARGB;

	if ( a_uiColour == ABlobColourComponent::COLOUR_BLACK )
	{
		m_uiFlags    = ( m_uiFlags & ~Flags_Painted ) | Flags_Inked;
		m_fInktTimer = s_cfFadeTime;
		ABuildingManager::GetSingleton()->AddActiveInstance( this );
		m_uiCurrentColour = ABlobColourComponent::COLOUR_WHITE;
		uiARGB            = ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_BLACK ];
		NotifyCivSpawners();
		RemoveJellyPass();
	}
	else if ( a_uiColour == ABlobColourComponent::COLOUR_WHITE )
	{
		if ( !a_bJelly || a_iPlayer < 0 || !( ( m_pTerrainInstance->m_uiFlags >> 24 ) & 1 ) )
		{
			m_uiFlags &= ~Flags_Inked;
			RemoveJellyPass();
		}
		else
		{
			m_uiFlags |= Flags_Inked;
			m_fInktTimer = s_cfFadeTime;
		}

		m_uiFlags &= ~Flags_Painted;
		m_uiCurrentColour = ABlobColourComponent::COLOUR_WHITE;
		uiARGB            = ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_WHITE ];
	}
	else
	{
		uiARGB            = ABlobColourComponent::ms_auiColours[ a_uiColour ];
		m_uiFlags         = ( m_uiFlags & ~Flags_Inked ) | Flags_Painted | Flags_PaintedOnce;
		m_uiCurrentColour = TUINT8( a_uiColour );

		if ( uiOldColour == ABlobColourComponent::COLOUR_WHITE )
			ABuildingManager::TriggerLinks( m_pTerrainInstance, AEntityIOPacket::MESSAGE_TRIGGER );

		for ( TINT i = 0; i < MAXBLOCKS; i++ )
		{
			if ( m_apBlocks[ i ] != TNULL && GetBlockType( m_apBlocks[ i ] ) != 7 )
				ABuildingManager::GetSingleton()->OnBlockBuildingPainted( this, a_iPlayer, *TSTATICCAST( PaintableBlock, m_apBlocks[ i ] ) );
		}
	}

	for ( TINT i = 0; i < MAXBLOCKS; i++ )
	{
		if ( !a_bUnk2 && m_apBlocks[ i ] != TNULL && GetBlockType( m_apBlocks[ i ] ) == 7 )
			ABuildingManager::GetSingleton()->RemoveJellyInstance( this );
	}

	AGameStatus::GetSingleton()->OnBuildingPaintChanged( this );
	AChallengeManager::GetSingleton()->OnPaintEvent( this );

	if ( a_bJelly && a_iPlayer >= 0 && ( ( m_pTerrainInstance->m_uiFlags >> 24 ) & 1 ) )
	{
		SetPaintColour( uiARGB, a_iPlayer, a_bUnk2, -1 );
		return;
	}

	SetLocalAmbientColour( uiARGB );
}

// $deBlob: FUNCTION 00577e00
void ABuildingInstance::SetBlock( void* a_pBlock )
{
	if ( m_apBlocks[ 0 ] == TNULL )
	{
		m_apBlocks[ 0 ] = a_pBlock;
	}
	else
	{
		TASSERT( TNULL == m_apBlocks[ 1 ] );
		m_apBlocks[ 1 ] = a_pBlock;
	}
}

// $deBlob: FUNCTION 00577e70
void ABuildingInstance::Unlock()
{
	TASSERT( m_uiFlags & Flags_Locked );
	TASSERT( !( m_uiFlags & Flags_Inked ) );
	TASSERT( !( m_uiFlags & Flags_Painted ) );
	TASSERT( !( m_uiFlags & Flags_PaintedOnce ) );
	TASSERT( ABlobColourComponent::COLOUR_WHITE == m_uiCurrentColour );

	m_uiFlags &= ~Flags_Locked;
	SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
}

// $deBlob: FUNCTION 00577fa0
void ABuildingInstance::Lock()
{
	TUINT8 uiColour = m_uiCurrentColour;
	m_uiFlags       = ( m_uiFlags & ~( Flags_Painted | Flags_Inked | Flags_PaintedOnce ) ) | Flags_Locked;

	if ( uiColour != ABlobColourComponent::COLOUR_WHITE )
	{
		static const TVector4 s_vOrigin( 0.0f, 0.0f, 0.0f, 1.0f );
		AGameStatus::GetSingleton()->GetLevelData().OnBuildingUnpainted( this, 0, TFALSE, &s_vOrigin, 0 );
	}

	m_uiCurrentColour = ABlobColourComponent::COLOUR_WHITE;
	SetCollisionLayers( TFALSE );
	RemoveJellyPass();
	SetLocalAmbientColour( *ABlobColourMaps::ms_pLockedBuildingColour );
}

// $deBlob: FUNCTION 00578090
void ABuildingInstance::RemoveJellyPass()
{
	if ( m_pTerrainInstance->m_uiFlags & 0x1000000 )
	{
		AJellyShader_DX11::GetSingleton()->OnInstanceRemoved( m_pTerrainInstance );

		if ( m_iGlintIndex != -1 )
			AGameStatus::GetSingleton()->FreeProjection( m_iGlintIndex );

		AGameStatus::RemoveBuildingProjection( this );

		TerrainTypes::InstanceMeshes* pMeshes = m_pTerrainInstance->m_pSymbolData;

		if ( m_uiUnkC6 != 0xFF && pMeshes->m_uiNumMeshes > 0 )
		{
			for ( TUINT i = 0; i < pMeshes->m_uiNumMeshes; i++ )
			{
				TWorldMaterialHAL* pMaterial = pMeshes->m_pMeshes[ i ].m_pMaterial;
				const TCHAR*       szName    = pMaterial->GetMaterialName();

				if ( szName == TNULL || ( TStringManager::String8Compare( szName, "window", 6 ) != 0 && TStringManager::String8Compare( szName, "sticker", 7 ) != 0 ) )
					pMaterial->SetTextureNum( pMaterial->GetTextureNum() - 1 );
			}
		}
	}

	m_uiUnkC6 = 0xFF;
}

// $deBlob: FUNCTION 00578000
void ABuildingInstance::SetCollisionLayers( TBOOL a_bRestore )
{
	ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, GetCollisionInstance() );

	if ( pCollision == TNULL )
		return;

	TINT iNumObjects = pCollision->GetNumPhysicsObjects();

	if ( iNumObjects <= 0 )
		return;

	TUINT32 uiBit = 1;

	for ( TINT i = 0; i < iNumObjects; i++ )
	{
		TPhysicsObjectContainer* pBody   = pCollision->GetPhysicsInfo( i ).m_pBody;
		TUINT32                  uiLayer = pBody ? ( pBody->m_uiCollisionFilterInfo & 0xfff ) : 0;

		if ( !a_bRestore )
		{
			if ( uiLayer & 8 )
			{
				m_uiCollisionLayerMask |= uiBit;
				TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, uiLayer & ~8 );
			}
		}
		else if ( m_uiCollisionLayerMask & uiBit )
		{
			TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, uiLayer | 8 );
		}

		uiBit = ( uiBit << 1 ) | ( uiBit >> 31 );
	}
}

// Sends MESSAGE_TRIGGER to the civilian spawners linked to the building (when it's inked)
// $deBlob: FUNCTION 005781a0
void ABuildingInstance::NotifyCivSpawners()
{
	ALandmarkInstance::Links* pLinks = TSTATICCAST( ALandmarkInstance::Links, m_pTerrainInstance->m_pLinks );

	if ( pLinks == TNULL )
		return;

	for ( TINT i = 0; i < pLinks->m_iNumEntities; i++ )
	{
		ABaseEntity* pEntity = ABaseEntity::FindIDEntity( pLinks->m_piEntityIDs[ i ] );

		if ( pEntity->IsA( &TGetClass( ACivSpawner ) ) )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pEntity->HandleEntityIO( TNULL, oPacket );
		}
	}
}

// $deBlob: FUNCTION 00578260
TUINT32 __stdcall ABuildingInstance::LerpColour( TUINT32 a_uiFrom, TUINT32 a_uiTo, TFLOAT a_fT )
{
	TUINT32 uiResult = 0;

	for ( TINT i = 0; i < 32; i += 8 )
	{
		TFLOAT fFrom = TFLOAT( TUINT8( a_uiFrom >> i ) );
		TFLOAT fTo   = TFLOAT( TUINT8( a_uiTo >> i ) );
		uiResult |= TUINT32( TINT( fFrom + ( fTo - fFrom ) * a_fT ) ) << i;
	}

	return uiResult;
}
