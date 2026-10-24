#include "pch.h"
#include "APropHazard.h"
#include "AContainerHazard.h"
#include "Entities/Helpers/ASpawnPointPool.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityModelComponent.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Collision/ACollisionInstance2.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "GameController/AGameStatus.h"
#include "Render/Material/AMaterialInfo.h"
#include "Render/Material/AMaterialManager.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Sound/APositionalSoundManager.h"
#include "Sound/ASoundInterface.h"
#include "ARandom.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/Havok/TPhysicsMeshContainer.h>
#include <Render/TModel.h>
#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Render/TSkeleton.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Shaders/WorldShader/TWorldShader.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Platform/DX11/TRender_DX11.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* APropHazard::sm_despawnEffect   = TNULL;
Toshi::TParticleEffect* APropHazard::sm_pElectricEffect = TNULL;

TSTATICASSERT( sizeof( APropHazard ) == 0xD4 );
TSTATICASSERT( sizeof( APropHazard::Tile ) == 0x1C );
TSTATICASSERT( sizeof( APropHazard::CollisionEntry ) == 0xC );

// The size of an electric tile and the half extents of the bounding boxes
static const TFLOAT s_fTileSize        = 6.0f;
static const TFLOAT s_fBBoxSmall       = 4.5f; // "PROP_rooftopelectric"
static const TFLOAT s_fBBoxSmallHeight = 0.5f;
static const TFLOAT s_fBBoxTile        = 6.0f;
static const TFLOAT s_fBBoxTileHeight  = 0.75f;

// The time of the hot plate's transition and the scroll speed of its noise pass
static const TFLOAT s_fTransitionTime = 1.0f;
static const TFLOAT s_fNoiseScroll    = 0.25f;

// The ambient colour scales of an electric tile, dark and flickering
static const TFLOAT s_fTileDarkScale    = 0.2f;
static const TFLOAT s_fTileFlickerScale = 0.4f;

// The distance fade of the hot plate (squared distances 173 and 260 from the camera)
static const TFLOAT s_fHotPlateFadeStartSq = 29929.0f;
static const TFLOAT s_fHotPlateFadeEndSq   = 67600.0f;

// The world shader's blend modes of the heat haze ("heat" meshes), of its packet and of the "hot" material
static const TWorldMaterial::BLENDMODE s_eBlendModeHeat       = TWorldMaterial::BLENDMODE_HEAT;
static const TWorldMaterial::BLENDMODE s_eBlendModeHeatPacket = 23;
static const TWorldMaterial::BLENDMODE s_eBlendModeHot        = 24;

// The heat haze strength (PS c18: the indirect scale and 2^exponent) the "heat" packets render with while the hot plate
// is on, and the world shader's default the packet restores (TWorldShader_DX11.cpp's s_fLightmapScale and
// s_iLightmapExponent, read directly by retail)
static TFLOAT       s_fHeatScale        = 0.5f;
static const TINT   s_iHeatExponent     = -5;
static const TFLOAT s_fHeatMaxScale     = 0.6f;
static const TFLOAT s_fDefaultHeatScale = 0.5f;
static const TINT   s_iDefaultHeatExpon = -6;
static TBOOL        s_bHeatScaleSet     = TFALSE;
static TFLOAT       s_fPrevHeatScale    = 0.0f;
static TUINT8       s_uiInkWallAlpha    = 0;

// $deBlob: FUNCTION 007b0fc0
// $deBlob: FUNCTION 007b0fd0
// $deBlob: FUNCTION 007b1040
// $deBlob: FUNCTION 007b10a0
// $deBlob: FUNCTION 007b10b0
// $deBlob: FUNCTION 00541640
// $deBlob: FUNCTION 0085a010
TDEFINE_CLASS( APropHazard );

// $deBlob: FUNCTION 007b10c0
APropHazard::APropHazard()
{
	m_pContainer           = TNULL;
	m_pSpawnPoints         = TNULL;
	m_pCollisionEntries    = TNULL;
	m_iNumCollisionEntries = 0;
	m_pTypeData            = TNULL;
	m_fTime                = 0.0f;
	m_uiHazardFlags        = 0;
	SetHash( TFALSE );
}

// $deBlob: FUNCTION 007b5ad0
// $deBlob: FUNCTION 007b1170
// $deBlob: FUNCTION 007b5ac3
APropHazard::~APropHazard()
{
	sm_despawnEffect   = TNULL;
	sm_pElectricEffect = TNULL;

	if ( m_pSpawnPoints != TNULL )
	{
		delete m_pSpawnPoints;
		m_pSpawnPoints = TNULL;
	}

	StopSound();

	switch ( GetHazardType() )
	{
		case TYPE_HOTPLATE:
		{
			HotPlateData* pData = TSTATICCAST( HotPlateData, m_pTypeData );

			if ( pData->m_pHotMaterial != TNULL )
			{
				pData->m_pHotMaterial->OnDestroy();
				delete pData->m_pHotMaterial;
			}

			if ( pData->m_pColdMaterial != TNULL )
			{
				pData->m_pColdMaterial->OnDestroy();
				delete pData->m_pColdMaterial;
			}

			delete pData;
			m_pTypeData = TNULL;
			break;
		}
		case TYPE_ELECTRIC:
		{
			ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

			if ( pData->m_pTiles != TNULL )
			{
				for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
				{
					if ( pData->m_pTiles[ i ].m_pModel != TNULL )
					{
						AModelFactory::GetSingleton()->DestroyInstance( pData->m_pTiles[ i ].m_pModel );
						pData->m_pTiles[ i ].m_pModel = TNULL;
					}

					if ( pData->m_pTiles[ i ].m_pEffect != TNULL )
					{
						pData->m_pTiles[ i ].m_pEffect->StopInstant();
						pData->m_pTiles[ i ].m_pEffect = TNULL;
					}
				}

				delete[] pData->m_pTiles;
				pData->m_pTiles = TNULL;
			}

			delete pData;
			m_pTypeData = TNULL;
			break;
		}
		case TYPE_SPIKES:
			KillManualAllAnimations( -1.0f );
			delete TSTATICCAST( SpikesData, m_pTypeData );
			m_pTypeData = TNULL;
			break;
		default:
			// NOTE: TYPE_INKWALL has no type data
			break;
	}

	delete[] m_pCollisionEntries;
}

// $deBlob: FUNCTION 007b1370
void APropHazard::Tick( TFLOAT a_fDeltaTime )
{
	AProp::Tick( a_fDeltaTime );

	STATE eState = GetHazardState();

	if ( eState == STATE_TURNINGON || eState == STATE_TURNINGOFF )
	{
		if ( GetHazardType() == TYPE_SPIKES )
		{
			SpikesData* pData = TSTATICCAST( SpikesData, m_pTypeData );

			if ( ( eState == STATE_TURNINGON && IsManualAnimationFinished( pData->m_iActivateAnim ) ) ||
			     ( GetHazardState() == STATE_TURNINGOFF && IsManualAnimationFinished( pData->m_iDeactivateAnim ) ) )
			{
				TSkeletonInstance* pSkeleton = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();
				pSkeleton->UpdateTime( 0.0f );
				pSkeleton->UpdateState( TFALSE );
			}
		}

		UpdateTransition( a_fDeltaTime );
	}

	if ( GetHazardType() == TYPE_HOTPLATE )
	{
		// Scrolls the noise (bump) pass of the hot material
		TWorldMaterialHAL* pHot = TDYNAMICCAST( TWorldMaterialHAL, TSTATICCAST( HotPlateData, m_pTypeData )->m_pHotMaterial );

		TFLOAT fOffset = s_fNoiseScroll * a_fDeltaTime + pHot->GetUOffset( 1 );
		pHot->SetUOffset( 1, fOffset );

		if ( 1.0f < fOffset )
			pHot->SetUOffset( 1, 0.0f );

		pHot->SetVOffset( 1, pHot->GetUOffset( 1 ) );
	}

	if ( GetHazardType() == TYPE_ELECTRIC )
	{
		if ( GetHazardState() == STATE_ON )
		{
			ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

			// Starts the sparks of each tile after its delay
			for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
			{
				Tile& rTile = pData->m_pTiles[ i ];

				if ( rTile.m_pEffect != TNULL )
					continue;

				rTile.m_fEffectDelay -= a_fDeltaTime;

				if ( 0.0f < rTile.m_fEffectDelay )
					continue;

				rTile.m_pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( sm_pElectricEffect );

				if ( rTile.m_pEffect != TNULL )
				{
					TMatrix44 mTransform;
					rTile.m_pModel->GetTransform( mTransform );

					TVector4 vOffset( 0.0f, -1.5f, 0.0f, 1.0f );
					TMatrix44::TransformVector( mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ), mTransform, vOffset );

					rTile.m_pEffect->GetTransform() = mTransform;
				}

				rTile.m_fEffectDelay = 0.0f;
			}
		}

		ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

		for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
			pData->m_pTiles[ i ].UpdateColour( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 007b1600
void APropHazard::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( TUINT8( m_Flags >> 24 ) & 1 )
		return;


	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TRIGGER )
	{
		Toggle();
		AProp::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}

	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_OPEN )
	{
		// On
		if ( GetHazardState() == STATE_OFF || GetHazardState() == STATE_TURNINGOFF )
		{
			SetState( STATE_TURNINGON );
			SetTicking( TTRUE );
		}
	}
	else if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_CLOSE )
	{
		// Off
		if ( GetHazardState() == STATE_ON || GetHazardState() == STATE_TURNINGON )
		{
			SetState( STATE_TURNINGOFF );
			SetTicking( TTRUE );
		}
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007b1690
// $deBlob: FUNCTION 007b5ba8
void APropHazard::ProcessParams( const AEntityParams& a_rParams )
{
	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );

	const TCHAR* szDespawnEffect = GetTypeData()->m_pTRBData->m_szDespawnEffect;

	if ( szDespawnEffect != TNULL && *szDespawnEffect != '\0' && sm_despawnEffect == TNULL )
	{
		sm_despawnEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( szDespawnEffect );
		TASSERT( TDebug_IsValidAddress( sm_despawnEffect ) && "Couldn't find despawn particle effect %s for hazard!" );
	}

	if ( sm_pElectricEffect == TNULL )
	{
		sm_pElectricEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_ElectricTrap" );
		TASSERT( TDebug_IsValidAddress( sm_pElectricEffect ) && "Couldn't find electricity effect for hazard!" );
	}

	AProp::ProcessParams( a_rParams );

	void* pPoints    = TNULL;
	TINT  iNumPoints = a_rParams.GetArray( pPoints, "HazardPointList" );

	if ( iNumPoints <= 0 )
		m_pSpawnPoints = TNULL;
	else
		m_pSpawnPoints = new ASpawnPointPool( TREINTERPRETCAST( TVector4* const&, pPoints ), TUINT8( iNumPoints ) );

	TBOOL bStartOn = TFALSE;
	a_rParams.GetValue( bStartOn, "StartOn" );

	if ( bStartOn )
		m_uiHazardFlags |= HAZARDFLAG_StartOn;
	else
		m_uiHazardFlags &= ~HAZARDFLAG_StartOn;

#ifdef DEBLOB_COMMUNITY_PATCH
	TINT iType = 0; // NOTE: not initialised in retail (a hazard without "Type" would read the stack)
#else
	TINT iType;
#endif
	a_rParams.GetValue( iType, "Type" );
	m_uiHazardFlags = ( m_uiHazardFlags & 0x3F ) | TUINT8( iType << 6 );

	switch ( iType & 3 )
	{
		case TYPE_HOTPLATE:
		{
			HotPlateData* pData = new HotPlateData;
			m_pTypeData         = pData;

			pData->m_pHotTexture  = TTextureManager::GetSingleton()->FindTexture( "prop_rooftophotplate.tga" );
			pData->m_pColdTexture = TTextureManager::GetSingleton()->FindTexture( "prop_rooftophotplateoff.tga" );

			AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
			m_fAmbientScale                 = 1.0f;
			pModelInstance->SetAmbientScale( 1.0f );
			pModelInstance->SetFlag( AModelInstance2::FLAG( 0x2000 ), TFALSE );

			{
				T2ModelPtr pModel = pModelInstance->GetModelInstance()->GetModel();
				TModelLOD& rLOD   = pModel->GetLOD( 0 );

				// Every mesh drawn as is; the "hotplate" material is the one both looks are copied from
				for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
				{
					TASSERT( TUINT8( i ) < 5 );
					pModelInstance->SetMeshFlags( TUINT8( i ), 1, TFALSE );

					TWorldMaterial* pMaterial = TSTATICCAST( TWorldMaterial, rLOD.GetMesh( i )->GetMaterial() );

					if ( TStringManager::String8Compare( pMaterial->GetMaterialName(), "hotplate", -1 ) == 0 )
						pData->m_pMaterial = pMaterial;
				}
			}

			TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

			// The on look: the hot texture and a scrolling noise (bump) pass, in the heat order table
			pData->m_pHotMaterial = TSTATICCAST( TWorldMaterialHAL, pShader->CreateMaterial( "hot" ) );
			pData->m_pHotMaterial->Create( s_eBlendModeHot );
			AMaterialInfo::CopyMaterial( pData->m_pMaterial, pData->m_pHotMaterial );
			pData->m_pHotMaterial->SetTexture( 0, pData->m_pHotTexture );

			TWorldMaterialHAL* pHot   = TDYNAMICCAST( TWorldMaterialHAL, pData->m_pHotMaterial );
			TTexture*          pNoise = TTextureManager::GetSingleton()->FindTexture( "hotplate_noise.tga" );

			TUINT32 iTexIndex = pHot->GetTextureNum();
			pHot->SetTextureNum( iTexIndex + 1 );
			pHot->SetTexture( iTexIndex, pNoise );
			pHot->SetMultiPassBlendType( iTexIndex, TWorldMaterialHAL::MULTIPASSBLENDTYPE_ADD );
			pHot->SetUVMode( iTexIndex, TWorldMaterialHAL::UVMODE_OFFSET );
			TASSERT( TWorldMaterial::TEXTURETYPE_BUMP == iTexIndex );
			pHot->SetOrderTable( pShader->GetOrderTable( 5 ), 0, TTRUE );

			// The off look
			pData->m_pColdMaterial = TSTATICCAST( TWorldMaterialHAL, pShader->CreateMaterial( "cold" ) );
			pData->m_pColdMaterial->Create( 1 );
			AMaterialInfo::CopyMaterial( pData->m_pMaterial, pData->m_pColdMaterial );
			pData->m_pColdMaterial->SetTexture( 0, pData->m_pColdTexture );
			pData->m_pColdMaterial->SetOrderTable( pShader->GetOrderTable( 4 ), 0, TTRUE );

			pModelInstance->GetModelInstance()->SetCustomRenderMethod( RenderModelCallback, this );
			AEntityFactory::GetSingleton()->AddCallback( this, RenderHeatCallback );
			break;
		}
		case TYPE_ELECTRIC:
		{
			ElectricData* pData     = new ElectricData;
			pData->m_pTiles         = TNULL;
			pData->m_iNumTiles      = 0;
			pData->m_iConfiguration = 0;
			m_pTypeData             = pData;

			pData->m_iConfiguration = 0;
			a_rParams.GetValue( pData->m_iConfiguration, "HazardConfiguration" );

			// The tile layout of the configuration: offsets in tiles (x, z) from the entity's matrix
			static const TINT s_aiTile1[] = { 0, 0 };
			static const TINT s_aiTile2[] = { -1, 0, 1, 0 };
			static const TINT s_aiTile3[] = { -1, -1, 1, -1, 1, 1, -1, 1 };
			static const TINT s_aiTile4[] = { -1, 1, 1, -1 };
			static const TINT s_aiTile5[] = { -2, 0, 0, 0, 2, 0 };
			static const TINT s_aiTile6[] = { -2, 1, 0, -1, 2, 1 };
			static const TINT s_aiTile7[] = { -2, 2, 2, 2, 0, 0, -2, -2, 2, -2 };
			static const TINT s_aiTile8[] = { 0, 2, -2, 0, 2, 0, 0, -2 };

			const TCHAR* szModel  = "model/prop/haz_ET_1x1.trb";
			const TINT*  pOffsets = TNULL;
			TINT         iNumTiles;

			switch ( pData->m_iConfiguration )
			{
				case 0:
					iNumTiles = 1;
					szModel   = "model/prop/PROP_rooftopelectric.trb";
					pOffsets  = s_aiTile1;
					break;
				case 1:
					iNumTiles = 1;
					pOffsets  = s_aiTile1;
					break;
				case 2:
					iNumTiles = 2;
					pOffsets  = s_aiTile2;
					break;
				case 3:
					iNumTiles = 4;
					pOffsets  = s_aiTile3;
					break;
				case 4:
					iNumTiles = 2;
					pOffsets  = s_aiTile4;
					break;
				case 5:
					iNumTiles = 3;
					pOffsets  = s_aiTile5;
					break;
				case 6:
					iNumTiles = 3;
					pOffsets  = s_aiTile6;
					break;
				case 7:
					iNumTiles = 5;
					pOffsets  = s_aiTile7;
					break;
				case 8:
					iNumTiles = 4;
					pOffsets  = s_aiTile8;
					break;
				default: iNumTiles = 0; break;
			}

			if ( pOffsets != TNULL )
			{
				pData->m_iNumTiles = iNumTiles;
				pData->m_pTiles    = new Tile[ iNumTiles ];

				for ( TINT i = 0; i < iNumTiles; i++ )
				{
					TMatrix44 mTile;

					// NOTE: the centre tiles use the entity's matrix as is, the others are moved by the offset
					if ( pOffsets[ i * 2 ] == 0 && pOffsets[ i * 2 + 1 ] == 0 )
					{
						mTile = *rEntityData.m_pMatrix;
					}
					else
					{
						mTile = *rEntityData.m_pMatrix;

						TVector4 vOffset( TFLOAT( pOffsets[ i * 2 ] ) * s_fTileSize, 0.0f, TFLOAT( pOffsets[ i * 2 + 1 ] ) * s_fTileSize, 1.0f );
						TMatrix44::TransformVector( mTile.AsBasisVector4( BASISVECTOR_TRANSLATION ), mTile, vOffset );
					}

					pData->m_pTiles[ i ].m_pModel = AModelFactory::GetSingleton()->CreateInstance( szModel, TTRUE, TTRUE );
					pData->m_pTiles[ i ].m_pModel->SetTransform( mTile );
				}
			}

			m_fAmbientScale = 1.0f;

			for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
			{
				pData->m_pTiles[ i ].m_pModel->SetAmbientScale( m_fAmbientScale );
				pData->m_pTiles[ i ].m_pModel->SetFlag( AModelInstance2::FLAG( 0x2000 ), TFALSE );
			}

			break;
		}
		case TYPE_SPIKES:
		{
			SpikesData* pData        = new SpikesData;
			pData->m_iActivateAnim   = -1;
			pData->m_iDeactivateAnim = -1;
			m_pTypeData              = pData;

			pData->m_iActivateAnim   = CreateManualAnimation( "anim_activate" );
			pData->m_iDeactivateAnim = CreateManualAnimation( "anim_deactivate" );
			break;
		}
		case TYPE_INKWALL:
		{
			AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
			pModelInstance->GetModelInstance()->SetCustomRenderMethod( RenderModelCallback, this );
			m_fAmbientScale = 1.0f;
			pModelInstance->SetAmbientScale( 1.0f );
			break;
		}
	}

	SetupCollision();
	CreateCollisionObject();
}

// $deBlob: FUNCTION 007b2f90
// $deBlob: FUNCTION 007b5b9d
void APropHazard::PostCreate()
{
	AProp::PostCreate();

	if ( GetHazardType() == TYPE_ELECTRIC )
	{
		TFLOAT fX;
		TFLOAT fY    = s_fBBoxTileHeight;
		TFLOAT fZ    = s_fBBoxTile;
		TBOOL  bBBox = TTRUE;

		switch ( TSTATICCAST( ElectricData, m_pTypeData )->m_iConfiguration )
		{
			case 0:
				fY = s_fBBoxSmallHeight;
				fZ = s_fBBoxSmall;
				fX = s_fBBoxSmall;
				break;
			case 1:
				fX = s_fBBoxTile;
				break;
			case 2:
				fX = s_fBBoxTile * 2.0f;
				break;
			case 3:
			case 4:
				fX = s_fBBoxTile * 2.0f;
				fZ = fX;
				break;
			case 5:
				fX = s_fBBoxTile * 3.0f;
				break;
			case 6:
				fZ = s_fBBoxTile * 2.0f;
				fX = s_fBBoxTile * 3.0f;
				break;
			case 7:
			case 8:
				fX = s_fBBoxTile * 3.0f;
				fZ = fX;
				break;
			default:
				bBBox = TFALSE;
				break;
		}

		if ( bBBox )
			SetBBox( TVector4( fX, fY, fZ, 1.0f ) );
	}

	if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint && ( m_uiHazardFlags & HAZARDFLAG_StartOn ) )
		SetState( STATE_TURNINGON );
	else
		SetState( STATE_TURNINGOFF );
}

// $deBlob: FUNCTION 007b3100
void APropHazard::Reset( ABaseEntity* a_pEntity )
{
	AProp::Reset( a_pEntity );

	if ( GetHazardType() == TYPE_SPIKES )
		KillManualAllAnimations( -1.0f );
	else if ( GetHazardType() == TYPE_ELECTRIC )
		OnTurnedOff();

	StopSound();

	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint || !( m_uiHazardFlags & HAZARDFLAG_StartOn ) )
		SetState( STATE_TURNINGOFF );
	else
		SetState( STATE_TURNINGON );

	if ( m_pSpawnPoints != TNULL )
		m_pSpawnPoints->ReleaseAll();
}

// $deBlob: FUNCTION 007b31b0
TBOOL APropHazard::CanBlobStick()
{
	if ( ( m_uiHazardFlags & HAZARDFLAG_TypeMask ) == ( TYPE_SPIKES << 6 ) )
		return TTRUE;

	return ( m_uiHazardFlags & HAZARDFLAG_StateMask ) == ( STATE_OFF << 4 );
}

// $deBlob: FUNCTION 007b31d0
void APropHazard::SetVisible( TBOOL a_bVisible )
{
	AVisibleEntity::SetVisible( a_bVisible );

	if ( GetHazardType() != TYPE_ELECTRIC )
		return;

	ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

	for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
	{
		pData->m_pTiles[ i ].m_pModel->SetFlag( AModelInstance2::FLAG( 1 ), a_bVisible );

		if ( !a_bVisible )
		{
			if ( pData->m_pTiles[ i ].m_pEffect != TNULL )
			{
				pData->m_pTiles[ i ].m_pEffect->StopInstant();
				pData->m_pTiles[ i ].m_pEffect = TNULL;
			}
		}
		else if ( pData->m_pTiles[ i ].m_pEffect == TNULL )
		{
			pData->m_pTiles[ i ].m_fEffectDelay = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( 0.0f, 0.1f );
		}
	}
}

// $deBlob: FUNCTION 007b32d0
void APropHazard::AddToPhysics()
{
	AVisibleEntity::AddToPhysics();

	if ( GetHazardType() != TYPE_ELECTRIC )
		return;

	ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

	for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
	{
		if ( pData->m_pTiles[ i ].m_pModel->GetCollisionInstance() != TNULL )
			TSTATICCAST( ACollisionInstance2, pData->m_pTiles[ i ].m_pModel->GetCollisionInstance() )->AddToWorld( 0x60 );
	}
}

// $deBlob: FUNCTION 007b3320
void APropHazard::RemoveFromPhysics()
{
	AVisibleEntity::RemoveFromPhysics();

	if ( GetHazardType() != TYPE_ELECTRIC )
		return;

	ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

	for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
	{
		if ( pData->m_pTiles[ i ].m_pModel->GetCollisionInstance() != TNULL )
			TSTATICCAST( ACollisionInstance2, pData->m_pTiles[ i ].m_pModel->GetCollisionInstance() )->RemoveFromWorld( 0x60 );
	}
}

// $deBlob: FUNCTION 007b3370
void __stdcall APropHazard::RenderModelCallback( TModelInstance* a_pModelInstance, void* a_pHazard )
{
	APropHazard*     pHazard = TSTATICCAST( APropHazard, a_pHazard );
	AModelInstance2* pThis   = TSTATICCAST( AEntityModelComponent, pHazard->GetModelComponent() )->GetModelInstance();

	T2ModelPtr         pModel            = a_pModelInstance->GetModel();
	TSkeletonInstance* pSkeletonInstance = a_pModelInstance->GetSkeletonInstance();

	if ( pSkeletonInstance->GetOverlayAnimationCount() + pSkeletonInstance->GetBaseAnimationCount() != 0 )
		pSkeletonInstance->UpdateState( TFALSE );

	TModelLOD& rLOD = pModel->GetLOD( a_pModelInstance->GetLOD() );

	if ( rLOD.GetMeshCount() < 1 )
		return;

	TBOOL  bWorld    = TFALSE;
	TBOOL  bSkinMesh = TFALSE;
	TUINT8 uiFlags   = TUINT8( pThis->GetFlags() );

	if ( uiFlags & 0x80 )
	{
		// Finds out once what kind of mesh the model has
		const TClass* pMeshClass = rLOD.GetMesh( 0 )->GetClass();
		bWorld                   = pMeshClass == &TGetClass( TWorldMeshHAL );

		if ( bWorld )
		{
			pThis->SetFlag( AModelInstance2::FLAG( 0x10 ), TTRUE );
		}
		else
		{
			bSkinMesh = rLOD.GetMesh( 0 )->GetClass() == &TGetClass( ASkinMeshHAL );

			if ( bSkinMesh )
				pThis->SetFlag( AModelInstance2::FLAG( 0x20 ), TTRUE );
		}

		pThis->SetFlag( AModelInstance2::FLAG( 0x80 ), TFALSE );
	}
	else if ( !( uiFlags & 0x10 ) )
	{
		if ( uiFlags & 0x20 )
			bSkinMesh = TTRUE;
	}
	else
	{
		bWorld = TTRUE;
	}

	TRender*        pRender  = TRender::GetSingleton();
	TRenderContext* pContext = pRender->GetCurrentContext();
	pContext->SetSkeletonInstance( pSkeletonInstance );

	const TVector4& rScale = pThis->GetScale();
	TBOOL           bScaled;
	TMatrix44       oOldModelView;

	if ( rScale.x == 1.0f && rScale.y == 1.0f && rScale.z == 1.0f && rScale.w == 1.0f )
	{
		bScaled = TFALSE;
	}
	else
	{
		bScaled = TTRUE;

		TRenderContext* pCurrentContext = pRender->GetCurrentContext();
		oOldModelView                   = pCurrentContext->GetModelViewMatrix();

		TMatrix44 oModelView = pCurrentContext->GetModelViewMatrix();
		oModelView.Scale( rScale );
		pCurrentContext->SetModelViewMatrix( oModelView );
	}

	AMaterialManager::GetSingleton()->UpdateUVs( a_pModelInstance->GetModel().GetData() );

	TMatrix44 oOldLightDirection = pRender->GetLightDirection();
	TMatrix44 oOldLightColour    = pRender->GetLightColour();
	TVector4  vOldAmbient        = pRender->GetCurrentContext()->GetAmbientColour();

	pThis->SetRenderLighting( pThis->m_fAmbientScale, TTRUE );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		TBOOL bMeshSettings = TFALSE;

		if ( ( pThis->GetFlags() & 0x1000 ) && i <= 4 )
		{
			TASSERT( TUINT8( i ) < AModelInstance2::MAX_MESH_SETTINGS );

			if ( pThis->m_aMeshSettings[ TUINT8( i ) ].uiFlags & 1 )
			{
				bMeshSettings = TTRUE;

				// Hidden mesh
				if ( !( pThis->m_aMeshSettings[ TUINT8( i ) ].uiFlags & 2 ) )
					continue;
			}
		}

		TBOOL        bHotPlate = TFALSE;
		TBOOL        bHeat     = TFALSE;
		TBOOL        bInkPool  = TFALSE;
		TBOOL        bRender   = TTRUE;
		const TCHAR* sMatName  = TNULL;
		TMesh*       pMesh     = rLOD.GetMesh( i );

		if ( bSkinMesh )
			sMatName = pMesh->GetMaterial()->GetName();
		else if ( bWorld )
			sMatName = TDYNAMICCAST( TWorldMaterial, pMesh->GetMaterial() )->GetMaterialName();

		TASSERT( TNULL != sMatName );

		if ( TStringManager::String8CompareNoCase( sMatName, "hotplate", -1 ) == 0 )
		{
			bHotPlate = TTRUE;
		}
		else if ( TStringManager::String8CompareNoCase( sMatName, "heat", -1 ) == 0 )
		{
			// The heat haze, drawn by RenderHeatCallback
			if ( bMeshSettings )
			{
				TWorldMaterialHAL* pWorldMat = TDYNAMICCAST( TWorldMaterialHAL, pMesh->GetMaterial() );
				TASSERT( pWorldMat->GetBlendMode() == s_eBlendModeHeat );
				bHeat = TTRUE;
			}

			bRender = TFALSE;
		}
		else if ( TStringManager::String8CompareNoCase( sMatName, "Ink_Pool", -1 ) == 0 )
		{
			bInkPool = TTRUE;
		}

		TRenderContext* pMeshContext = TRender::GetSingleton()->GetCurrentContext();

		if ( bHeat )
		{
			TASSERT( TTRUE == bWorld );
			TSTATICCAST( TWorldMesh, pMesh )->RenderWithCallback( RenderPacketCallback, pHazard );
		}
		else if ( bMeshSettings )
		{
			TFLOAT   fOldAlpha       = pMeshContext->GetAlphaBlend();
			TVector4 vOldMeshAmbient = pMeshContext->GetAmbientColour();

			TASSERT( TUINT8( i ) < AModelInstance2::MAX_MESH_SETTINGS );
			pMeshContext->SetAlphaBlend( TFLOAT( pThis->m_aMeshSettings[ TUINT8( i ) ].aColour[ 3 ] ) / 255.0f );

			TVector4 vColour;
			UnpackColour( *TREINTERPRETCAST( TUINT32*, pThis->m_aMeshSettings[ TUINT8( i ) ].aColour ), vColour );
			pMeshContext->SetAmbientColour( vColour );

			pMesh->Render();

			pMeshContext->SetAlphaBlend( fOldAlpha );
			pMeshContext->SetAmbientColour( vOldMeshAmbient );
		}
		else if ( bHotPlate )
		{
			TASSERT( TTRUE == bWorld );

			// Fades out between 173 and 260 from the camera
			ACamera* pCamera   = ACameraManager::GetSingleton()->GetCurrentCamera();
			TFLOAT   fDistance = TVector4::DistanceSq( pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), pHazard->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );
			TFLOAT   fAlpha;

			if ( fDistance <= s_fHotPlateFadeStartSq )
				fAlpha = 1.0f;
			else
				fAlpha = TMath::LerpTowards( 1.0f, 0.0f, ( fDistance - s_fHotPlateFadeStartSq ) / ( s_fHotPlateFadeEndSq - s_fHotPlateFadeStartSq ) );

			TFLOAT fOldAlpha = pMeshContext->GetAlphaBlend();
			pMeshContext->SetAlphaBlend( fAlpha );
			pHazard->RenderHotPlateMesh( TSTATICCAST( TWorldMesh, pMesh ), pMeshContext );
			pMeshContext->SetAlphaBlend( fOldAlpha );
		}
		else if ( bInkPool )
		{
			TASSERT( TTRUE == bWorld );
			pHazard->RenderInkWallMesh( TSTATICCAST( TWorldMesh, pMesh ), pMeshContext );
		}
		else if ( bRender )
		{
			pMesh->Render();
		}
	}

	if ( bScaled )
		pRender->GetCurrentContext()->SetModelViewMatrix( oOldModelView );

	pRender->GetCurrentContext()->SetAmbientColour( vOldAmbient );
	pRender->SetLightDirectionMatrix( oOldLightDirection );
	pRender->SetLightColourMatrix( oOldLightColour );
}

// $deBlob: FUNCTION 007b3bc0
void APropHazard::RenderHotPlateMesh( TWorldMesh* a_pMesh, TRenderContext* a_pContext )
{
	HotPlateData* pData     = TSTATICCAST( HotPlateData, m_pTypeData );
	TFLOAT        fProgress = m_fTime / m_fDuration;

	TWorldMaterial* pMaterial = TDYNAMICCAST( TWorldMaterial, a_pMesh->GetMaterial() );

	switch ( GetHazardState() )
	{
		case STATE_TURNINGON:
		{
			// The cold look fades out under the hot one
			TFLOAT fOldAlpha = a_pContext->GetAlphaBlend();
			a_pContext->SetAlphaBlend( 1.0f - fProgress );
			a_pMesh->SetMaterial( pData->m_pColdMaterial, TFALSE );
			a_pMesh->Render();
			a_pContext->SetAlphaBlend( fProgress );
			a_pMesh->SetMaterial( pData->m_pHotMaterial, TFALSE );
			a_pMesh->Render();
			a_pMesh->SetMaterial( pMaterial, TFALSE );
			a_pContext->SetAlphaBlend( fOldAlpha );
			return;
		}
		case STATE_TURNINGOFF:
		{
			TFLOAT fOldAlpha = a_pContext->GetAlphaBlend();
			a_pContext->SetAlphaBlend( fProgress );
			a_pMesh->SetMaterial( pData->m_pColdMaterial, TFALSE );
			a_pMesh->Render();
			a_pContext->SetAlphaBlend( 1.0f - fProgress );
			a_pMesh->SetMaterial( pData->m_pHotMaterial, TFALSE );
			a_pMesh->Render();
			a_pMesh->SetMaterial( pMaterial, TFALSE );
			a_pContext->SetAlphaBlend( fOldAlpha );
			return;
		}
		case STATE_ON:
			a_pMesh->SetMaterial( pData->m_pHotMaterial, TFALSE );
			break;
		case STATE_OFF:
			a_pMesh->SetMaterial( pData->m_pColdMaterial, TFALSE );
			break;
	}

	a_pMesh->Render();
	a_pMesh->SetMaterial( pMaterial, TFALSE );
}

// $deBlob: FUNCTION 007b3d60
void APropHazard::RenderHeatMesh( TWorldMesh* a_pMesh, TRenderContext* a_pContext )
{
	a_pMesh->RenderWithCallback( RenderPacketCallback, this );
}

// $deBlob: FUNCTION 007b3d80
void APropHazard::RenderInkWallMesh( TWorldMesh* a_pMesh, TRenderContext* a_pContext )
{
	TFLOAT fAlpha;

	switch ( GetHazardState() )
	{
		case STATE_TURNINGON:
			fAlpha = m_fTime / m_fDuration;
			break;
		case STATE_TURNINGOFF:
			fAlpha = 1.0f - m_fTime / m_fDuration;
			break;
		case STATE_ON:
			a_pMesh->Render();
			return;
		default:
			return;
	}

	TFLOAT fOldAlpha = a_pContext->GetAlphaBlend();
	a_pContext->SetAlphaBlend( fAlpha );
	a_pMesh->RenderWithCallback( RenderPacketCallback, this );
	a_pContext->SetAlphaBlend( fOldAlpha );
}

// $deBlob: FUNCTION 007b3e20
void APropHazard::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( !( m_Flags & FLAG_Despawned ) )
	{
		if ( a_iLOD & FLAG_LowLOD )
			StopSound();
		else if ( GetHazardState() == STATE_ON )
			StartSound();
	}

	if ( a_iLOD & FLAG_HighLOD )
	{
		if ( GetHazardType() == TYPE_ELECTRIC )
		{
			ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

			for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
				pData->m_pTiles[ i ].m_pModel->SetFlag( AModelInstance2::FLAG( 1 ), ( m_Flags >> 2 ) & 1 );
		}
	}
	else if ( GetHazardType() == TYPE_ELECTRIC )
	{
		ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

		for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
			pData->m_pTiles[ i ].m_pModel->SetFlag( AModelInstance2::FLAG( 1 ), TFALSE );
	}

	if ( a_iLOD & FLAG_LowLOD )
	{
		RemoveFromPhysics();
		return;
	}

	if ( ( GetLOD() & FLAG_LowLOD ) && ( TUINT8( m_Flags >> 2 ) & 1 ) )
		AddToPhysics();
}

// $deBlob: FUNCTION 007b3f70
void APropHazard::RenderHeatCallback( void* a_pHazard )
{
	APropHazard*     pHazard = TSTATICCAST( APropHazard, a_pHazard );
	AModelInstance2* pThis   = TSTATICCAST( AEntityModelComponent, pHazard->GetModelComponent() )->GetModelInstance();

	T2ModelPtr pModel = pThis->GetModelInstance()->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( pThis->GetModelInstance()->GetLOD() );

	// Only the "heat" meshes, in the heat haze blend mode
	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		TWorldMaterial* pMaterial = TDYNAMICCAST( TWorldMaterial, rLOD.GetMesh( i )->GetMaterial() );
		const TCHAR*    sMatName  = pMaterial->GetMaterialName();
		TASSERT( TNULL != sMatName );

		if ( TStringManager::String8CompareNoCase( sMatName, "heat", -1 ) == 0 )
		{
			TDYNAMICCAST( TWorldMaterialHAL, rLOD.GetMesh( i )->GetMaterial() )->SetBlendMode( s_eBlendModeHeat );

			TASSERT( TUINT8( i ) < AModelInstance2::MAX_MESH_SETTINGS );
			pThis->SetMeshFlags( TUINT8( i ), 1, TTRUE );
			pThis->SetMeshFlags( TUINT8( i ), 2, TTRUE );
		}
		else
		{
			TASSERT( TUINT8( i ) < AModelInstance2::MAX_MESH_SETTINGS );
			pThis->SetMeshFlags( TUINT8( i ), 1, TTRUE );
			pThis->SetMeshFlags( TUINT8( i ), 2, TFALSE );
		}
	}

	pThis->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
	pThis->Render();
	pThis->SetFlag( AModelInstance2::FLAG( 0x1000 ), TFALSE );
}

// $deBlob: FUNCTION 007b4270
void __stdcall APropHazard::RenderPacketCallback( TRenderPacket* a_pPacket, void* a_pHazard, TBOOL a_bBefore )
{
	APropHazard* pHazard = TSTATICCAST( APropHazard, a_pHazard );

	if ( pHazard->GetHazardType() == TYPE_HOTPLATE )
	{
		if ( a_bBefore )
		{
			// The heat haze grows and shrinks with the transition
			if ( TDYNAMICCAST( TWorldMaterialHAL, a_pPacket->GetMaterial() )->GetBlendMode() == s_eBlendModeHeatPacket )
			{
				TFLOAT fScale;

				switch ( pHazard->GetHazardState() )
				{
					case STATE_TURNINGON:
						fScale = TMath::LERP( 0.0f, s_fHeatMaxScale, pHazard->m_fTime / pHazard->m_fDuration );
						break;
					case STATE_TURNINGOFF:
						fScale = TMath::LERP( s_fHeatMaxScale, 0.0f, pHazard->m_fTime / pHazard->m_fDuration );
						break;
					case STATE_ON:
						fScale = s_fHeatMaxScale;
						break;
					default:
						fScale = 0.0f;
						break;
				}

				if ( fScale != s_fHeatScale && fScale != s_fDefaultHeatScale )
				{
					s_fPrevHeatScale = s_fHeatScale;
					s_fHeatScale     = fScale;

					TVector4 vIndirect( fScale, TFLOAT( pow( 2.0, TFLOAT( s_iHeatExponent ) ) ), 0.0f, 0.0f );
					TRenderDX11::Interface()->SetVec4InPSBuffer( 0x12, &vIndirect, 1 );
					s_bHeatScaleSet = TTRUE;
				}
			}
		}
		else if ( s_bHeatScaleSet == TTRUE )
		{
			// The world shader's default
			TVector4 vIndirect( s_fDefaultHeatScale, TFLOAT( pow( 2.0, TFLOAT( s_iDefaultHeatExpon ) ) ), 0.0f, 0.0f );
			TRenderDX11::Interface()->SetVec4InPSBuffer( 0x12, &vIndirect, 1 );
			s_fHeatScale    = s_fPrevHeatScale;
			s_bHeatScaleSet = TFALSE;
		}
	}
	else if ( pHazard->GetHazardType() == TYPE_INKWALL )
	{
		TWorldMaterialHAL* pMaterial = TSTATICCAST( TWorldMaterialHAL, a_pPacket->GetMaterial() );

		if ( !a_bBefore )
		{
			pMaterial->SetAlpha( s_uiInkWallAlpha );
		}
		else if ( pHazard->GetHazardState() == STATE_TURNINGON || pHazard->GetHazardState() == STATE_TURNINGOFF )
		{
			s_uiInkWallAlpha = pMaterial->GetAlpha();
			pMaterial->SetAlpha( 1 );
		}
	}
}

// $deBlob: FUNCTION 007b44d0
void* APropHazard::GetSpawnPoint()
{
	TUINT uiOwnPoints = 0;

	if ( m_pSpawnPoints != TNULL )
		uiOwnPoints = m_pSpawnPoints->GetNumFreePoints();

	TUINT uiNumPoints = uiOwnPoints;

	if ( m_pContainer != TNULL && m_pContainer->GetSpawnPoints() != TNULL )
		uiNumPoints = uiOwnPoints + m_pContainer->GetSpawnPoints()->GetNumFreePoints();

	if ( uiNumPoints == 0 )
		return TNULL;

	// One of all the free points: the own ones first
	if ( ARandom::GetSingleton()->GetRandom().GetInt( uiNumPoints ) < TINT( uiOwnPoints ) )
		return m_pSpawnPoints->GetSpawnPoint();

	return m_pContainer->GetSpawnPoints()->GetSpawnPoint();
}

// $deBlob: FUNCTION 007b4540
void APropHazard::Despawn( TBOOL a_bPlayEffect )
{
	StopSound();

	if ( GetHazardType() == TYPE_SPIKES )
		KillManualAllAnimations( -1.0f );

	AVisibleEntity::Despawn( a_bPlayEffect );
}

// $deBlob: FUNCTION 007b45c0
void APropHazard::Toggle()
{
	switch ( GetHazardState() )
	{
		case STATE_ON:
		case STATE_TURNINGON:
			SetState( STATE_TURNINGOFF );
			SetTicking( TTRUE );
			return;
		case STATE_TURNINGOFF:
		case STATE_OFF:
			SetState( STATE_TURNINGON );
	}

	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007b4610
void APropHazard::SetOn( TBOOL a_bOn )
{
	STATE eState = GetHazardState();

	if ( !a_bOn )
	{
		if ( eState == STATE_ON || eState == STATE_TURNINGON )
		{
			SetState( STATE_TURNINGOFF );
			SetTicking( TTRUE );
		}
	}
	else if ( eState == STATE_OFF || eState == STATE_TURNINGOFF )
	{
		SetState( STATE_TURNINGON );
		SetTicking( TTRUE );
	}
}

// $deBlob: FUNCTION 007b4670
void APropHazard::UpdateTransition( TFLOAT a_fDeltaTime )
{
	STATE eState = GetHazardState();

	if ( eState == STATE_TURNINGON )
	{
		switch ( GetHazardType() )
		{
			case TYPE_HOTPLATE:
			case TYPE_INKWALL:
				m_fTime = m_fTime + a_fDeltaTime;

				if ( m_fDuration <= m_fTime )
				{
					OnTurnedOn();
					return;
				}

				break;
			case TYPE_ELECTRIC:
			{
				ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );
				TINT          iDone = 0;
				m_fTime             = m_fTime + a_fDeltaTime;

				for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
				{
					if ( pData->m_pTiles[ i ].UpdateTurningOn( a_fDeltaTime ) )
						iDone++;
				}

				if ( pData->m_iNumTiles <= iDone )
				{
					OnTurnedOn();
					return;
				}

				break;
			}
			case TYPE_SPIKES:
				if ( IsManualAnimationFinished( TSTATICCAST( SpikesData, m_pTypeData )->m_iActivateAnim ) )
				{
					OnTurnedOn();
					return;
				}

				break;
		}
	}
	else if ( eState == STATE_TURNINGOFF )
	{
		TYPE eType = GetHazardType();

		if ( eType == TYPE_SPIKES )
		{
			if ( !IsManualAnimationFinished( TSTATICCAST( SpikesData, m_pTypeData )->m_iDeactivateAnim ) )
				return;

			OnTurnedOff();
			SetTicking( TFALSE );
			return;
		}

		if ( eType == TYPE_HOTPLATE || eType == TYPE_INKWALL )
		{
			m_fTime = m_fTime + a_fDeltaTime;

			if ( m_fDuration <= m_fTime )
				OnTurnedOff();
		}
	}
}

// $deBlob: FUNCTION 007b4800
void APropHazard::OnTurnedOff()
{
	StopSound();

	// Removes the hazard's surface flag from the collision
	TUINT32 uiSurface;

	switch ( GetHazardType() )
	{
		case TYPE_HOTPLATE: uiSurface = ACollisionCommon::SURFACE_Hot; break;
		case TYPE_ELECTRIC: uiSurface = ACollisionCommon::SURFACE_Electric; break;
		case TYPE_SPIKES: uiSurface = ACollisionCommon::SURFACE_Spikes; break;
		default: uiSurface = ACollisionCommon::SURFACE_InkWall; break;
	}

	for ( TINT i = 0; i < m_iNumCollisionEntries; i++ )
	{
		CollisionEntry& rEntry  = m_pCollisionEntries[ i ];
		TUINT32         uiFlags = rEntry.m_uiFlags;

		if ( uiFlags & uiSurface )
		{
			uiFlags &= ~uiSurface;

			if ( rEntry.m_pBody == TNULL )
			{
				if ( rEntry.m_pMaterial != TNULL )
					rEntry.m_pMaterial->m_uiSurface = uiFlags;
			}
			else
			{
				rEntry.m_pBody->m_uiSurface = uiFlags;
			}
		}
	}

	if ( GetHazardType() == TYPE_ELECTRIC )
	{
		ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

		for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
			pData->m_pTiles[ i ].TurnOff();
	}
	else if ( GetHazardType() == TYPE_SPIKES )
	{
		SpikesData* pData = TSTATICCAST( SpikesData, m_pTypeData );

		// Holds the end of the deactivate animation
		if ( pData->m_iDeactivateAnim != -1 )
		{
			KillManualAllAnimations( -1.0f );
			AddManualAnimation( pData->m_iDeactivateAnim, -1.0f, 0.0f, 1.0f, 1.0f );

			TSkeletonInstance* pSkeleton = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();
			pSkeleton->UpdateTime( 0.0f );
			pSkeleton->UpdateState( TFALSE );
		}
	}

	SetState( STATE_OFF );
	SetTicking( TFALSE );
}

// $deBlob: FUNCTION 007b4ab0
void APropHazard::OnTurnedOn()
{
	StartSound();

	// Restores the surface flags of the collision
	for ( TINT i = 0; i < m_iNumCollisionEntries; i++ )
	{
		CollisionEntry& rEntry = m_pCollisionEntries[ i ];

		if ( rEntry.m_pBody == TNULL )
		{
			if ( rEntry.m_pMaterial != TNULL )
				rEntry.m_pMaterial->m_uiSurface = rEntry.m_uiFlags;
		}
		else
		{
			rEntry.m_pBody->m_uiSurface = rEntry.m_uiFlags;
		}
	}

	SetState( STATE_ON );

	if ( GetHazardType() == TYPE_ELECTRIC )
	{
		ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );

		for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
			pData->m_pTiles[ i ].TurnOn();
	}
	else if ( GetHazardType() == TYPE_SPIKES && TSTATICCAST( SpikesData, m_pTypeData )->m_iActivateAnim != -1 )
	{
		// Holds the end of the activate animation
		KillManualAllAnimations( -1.0f );
		AddManualAnimation( TSTATICCAST( SpikesData, m_pTypeData )->m_iActivateAnim, -1.0f, 0.0f, 1.0f, 1.0f );

		TSkeletonInstance* pSkeleton = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();
		pSkeleton->UpdateTime( 0.0f );
		pSkeleton->UpdateState( TFALSE );
		SetTicking( TFALSE );
		return;
	}

	SetTicking( TFALSE );
}

// $deBlob: FUNCTION 007b4c60
void APropHazard::StartSound()
{
	if ( m_uiHazardFlags & HAZARDFLAG_Sound )
		return;

	m_uiHazardFlags |= HAZARDFLAG_Sound;

	switch ( GetHazardType() )
	{
		case TYPE_HOTPLATE:
			APositionalSoundManager::GetSingleton()->AddEntitySound( this, 5, 0x5E );
			return;
		case TYPE_ELECTRIC:
			APositionalSoundManager::GetSingleton()->AddEntitySound( this, 5, 0x55 );
			return;
		case TYPE_INKWALL:
			APositionalSoundManager::GetSingleton()->AddEntitySound( this, 8, 0x8C );
			return;
		default:
			// No sound for the spikes
			m_uiHazardFlags &= ~HAZARDFLAG_Sound;
			return;
	}
}

// $deBlob: FUNCTION 007b4cd0
void APropHazard::StopSound()
{
	if ( !( m_uiHazardFlags & HAZARDFLAG_Sound ) )
		return;

	switch ( GetHazardType() )
	{
		case TYPE_HOTPLATE:
			APositionalSoundManager::GetSingleton()->RemoveSound( this, 5, 0x5E );
			break;
		case TYPE_ELECTRIC:
			APositionalSoundManager::GetSingleton()->RemoveSound( this, 5, 0x55 );
			break;
		case TYPE_INKWALL:
			APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x8C );
			break;
		default:
			break;
	}

	m_uiHazardFlags &= ~HAZARDFLAG_Sound;
}

// $deBlob: FUNCTION 007b4d20
void APropHazard::SetState( STATE a_eState )
{
	TINT iSound = -1;

	if ( a_eState == STATE_TURNINGON )
	{
		switch ( GetHazardType() )
		{
			case TYPE_HOTPLATE:
				// Continues an interrupted turning off backwards
				if ( GetHazardState() == STATE_TURNINGOFF )
				{
					m_fTime = m_fDuration - m_fTime;
				}
				else
				{
					m_fTime     = 0.0f;
					m_fDuration = s_fTransitionTime;
				}

				iSound = 0x5D;
				break;
			case TYPE_ELECTRIC:
			{
				// The tiles flicker on, each for its own time
				ElectricData* pData = TSTATICCAST( ElectricData, m_pTypeData );
				m_fTime             = 0.0f;
				m_fDuration         = 0.0f;

				for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
				{
					TFLOAT fDuration = pData->m_pTiles[ i ].StartTurningOn();
					m_fDuration      = ( fDuration <= m_fDuration ) ? m_fDuration : fDuration;
				}

				break;
			}
			case TYPE_SPIKES:
			{
				SpikesData* pData = TSTATICCAST( SpikesData, m_pTypeData );

				if ( pData->m_iActivateAnim == -1 )
				{
					OnTurnedOn();
					return;
				}

				KillManualAllAnimations( -1.0f );
				AddManualAnimation( pData->m_iActivateAnim, 0.0f, 0.0f, 1.0f, 1.0f );
				SetTicking( TTRUE );
				iSound = 100;
				break;
			}
			case TYPE_INKWALL:
				if ( GetHazardState() == STATE_TURNINGOFF )
				{
					m_fTime = m_fDuration - m_fTime;
				}
				else
				{
					m_fTime     = 0.0f;
					m_fDuration = s_fTransitionTime;
				}

				break;
		}
	}
	else if ( a_eState == STATE_TURNINGOFF )
	{
		switch ( GetHazardType() )
		{
			case TYPE_HOTPLATE:
				if ( GetHazardState() == STATE_TURNINGON )
				{
					m_fTime = m_fDuration - m_fTime;
				}
				else
				{
					m_fTime     = 0.0f;
					m_fDuration = s_fTransitionTime;
				}

				iSound = 0x5C;
				break;
			case TYPE_ELECTRIC:
				// At once
				OnTurnedOff();
				return;
			case TYPE_SPIKES:
			{
				SpikesData* pData = TSTATICCAST( SpikesData, m_pTypeData );

				if ( pData->m_iDeactivateAnim == -1 )
				{
					OnTurnedOff();
					SetTicking( TFALSE );
					return;
				}

				KillManualAllAnimations( -1.0f );
				AddManualAnimation( pData->m_iDeactivateAnim, 0.0f, 0.0f, 1.0f, 1.0f );
				SetTicking( TTRUE );
				iSound = 99;
				break;
			}
			case TYPE_INKWALL:
				if ( GetHazardState() == STATE_TURNINGON )
				{
					m_fTime = m_fDuration - m_fTime;
				}
				else
				{
					m_fTime     = 0.0f;
					m_fDuration = s_fTransitionTime;
				}

				break;
		}
	}

	if ( iSound != -1 && GetLOD() != FLAG_LowLOD )
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 5 ), ASoundMap::EVENTID( iSound ), GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), 0 );

	m_uiHazardFlags = ( m_uiHazardFlags & 0xCF ) | TUINT8( a_eState << 4 );
}

// $deBlob: FUNCTION 007b4ff0
void APropHazard::SetHazardType( TYPE a_eType )
{
	m_uiHazardFlags = ( m_uiHazardFlags & 0x3F ) | TUINT8( a_eType << 6 );
}

// Counts the bodies of a_pCollision and their mesh materials
static TINT CountCollisionEntries( ACollisionInstance2* a_pCollision )
{
	TINT iCount = a_pCollision->GetNumPhysicsObjects();

	for ( TINT i = 0; i < a_pCollision->GetNumPhysicsObjects(); i++ )
	{
		TPhysicsObjectContainer* pBody          = a_pCollision->GetPhysicsInfo( i ).m_pBody;
		TPhysicsMeshContainer*   pMeshContainer = pBody ? pBody->m_pMeshContainer : TNULL;
		iCount += pMeshContainer->GetNumMaterials();
	}

	return iCount;
}

// Fills the entries of the bodies of a_pCollision and their mesh materials from a_iIndex
static TINT AddCollisionEntries( APropHazard::CollisionEntry* a_pEntries, TINT a_iIndex, ACollisionInstance2* a_pCollision )
{
	for ( TINT i = 0; i < a_pCollision->GetNumPhysicsObjects(); i++ )
	{
		TPhysicsObjectContainer* pBody = a_pCollision->GetPhysicsInfo( i ).m_pBody;

		a_pEntries[ a_iIndex ].m_pBody     = pBody;
		a_pEntries[ a_iIndex ].m_pMaterial = TNULL;
		// TPhysicsManager::GetDefaultMaterial
		TASSERT( pBody != TNULL );
		a_pEntries[ a_iIndex ].m_uiFlags = pBody ? pBody->m_uiSurface : 0;
		a_iIndex++;

		TPhysicsMeshContainer* pMeshContainer = pBody ? pBody->m_pMeshContainer : TNULL;

		for ( TINT j = 0; j < pMeshContainer->GetNumMaterials(); j++ )
		{
			a_pEntries[ a_iIndex ].m_pBody = TNULL;

			TPhysicsMeshMaterial* pMaterial = TNULL;

			if ( pBody != TNULL && pBody->m_eShapeType == 6 )
				pMaterial = pMeshContainer->GetMaterial( j );

			a_pEntries[ a_iIndex ].m_pMaterial = pMaterial;
			a_pEntries[ a_iIndex ].m_uiFlags   = pMaterial ? pMaterial->m_uiSurface : 0;
			a_iIndex++;
		}
	}

	return a_iIndex;
}

// $deBlob: FUNCTION 007b5010
void APropHazard::SetupCollision()
{
	if ( GetHazardType() == TYPE_ELECTRIC )
	{
		ElectricData* pData    = TSTATICCAST( ElectricData, m_pTypeData );
		m_iNumCollisionEntries = 0;

		// The collision of every tile
		for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
		{
			AModelInstance2* pModel         = pData->m_pTiles[ i ].m_pModel;
			TModelInstance*  pModelInstance = pModel->GetModelInstance();
			T2ModelPtr       pTRBModel      = pModelInstance->GetModel();

			ACollisionInstance2* pCollision = new ACollisionInstance2();
			pCollision->SetModelInstance( pModelInstance, TNULL );

			ACollisionInstance2::OwnerInfo oOwner;
			oOwner.m_eType  = 2;
			oOwner.m_pOwner = this;
			pCollision->Create( pModelInstance, &oOwner, TFALSE, 0, TNULL );
			pModel->SetCollisionInstance( pCollision );

			TMatrix44 mTransform;
			pModel->GetTransform( mTransform );
			pCollision->SetMatrix( mTransform );

			m_iNumCollisionEntries += CountCollisionEntries( pCollision );
		}

		if ( 0 < m_iNumCollisionEntries )
		{
			m_pCollisionEntries = new CollisionEntry[ m_iNumCollisionEntries ];
			TINT iIndex         = 0;

			for ( TINT i = 0; i < pData->m_iNumTiles; i++ )
				iIndex = AddCollisionEntries( m_pCollisionEntries, iIndex, TSTATICCAST( ACollisionInstance2, pData->m_pTiles[ i ].m_pModel->GetCollisionInstance() ) );
		}
	}
	else
	{
		CreateModelCollision( TFALSE );

		ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetCollisionInstance() );
		m_iNumCollisionEntries          = CountCollisionEntries( pCollision );

		if ( 0 < m_iNumCollisionEntries )
		{
			m_pCollisionEntries = new CollisionEntry[ m_iNumCollisionEntries ];
			AddCollisionEntries( m_pCollisionEntries, 0, pCollision );
		}
	}

	if ( GetHazardType() == TYPE_SPIKES )
	{
		// Toggles the decal geometry layer of the bodies
		ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetCollisionInstance() );

		for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
		{
			TPhysicsObjectContainer* pBody          = pCollision->GetPhysicsInfo( i ).m_pBody;
			TUINT32                  iCollisionFlag = TPhysicsManager::GetCollisionLayer( pBody );

			if ( !( iCollisionFlag & 8 ) )
			{
				TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, iCollisionFlag | 8 );
			}
			else
			{
				TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, iCollisionFlag & ~8 );
				TASSERT( ( ( iCollisionFlag & ~8 ) & ~8 ) == 0 );
			}
		}
	}
}

// $deBlob: FUNCTION 007b56e0
void APropHazard::Tile::UpdateColour( TFLOAT a_fDeltaTime )
{
	m_pModel->UseLocalAmbientColour( TTRUE );

	TUINT32 uiColour = AModelInstance2::ms_uiDefaultLocalAmbientColour;
	TFLOAT  fScale;

	if ( !( m_uiFlags & TILEFLAG_Lit ) )
	{
		fScale = s_fTileDarkScale;
	}
	else
	{
		if ( !( m_uiFlags & TILEFLAG_TurningOn ) )
		{
			m_pModel->SetLocalAmbientColour( uiColour );
			m_pModel->UseLocalAmbientLight( TFALSE );
			return;
		}

		fScale = s_fTileFlickerScale;
	}

	TUINT32 uiR = TMath::TruncToInt( TFLOAT( ( uiColour >> 16 ) & 0xFF ) * fScale );
	TUINT32 uiG = TMath::TruncToInt( TFLOAT( ( uiColour >> 8 ) & 0xFF ) * fScale );
	TUINT32 uiB = TMath::TruncToInt( TFLOAT( uiColour & 0xFF ) * fScale );

	TUINT32 uiScaled = uiB | ( ( ( ( uiColour >> 24 ) << 8 | uiR ) << 8 | uiG ) << 8 );
	m_pModel->SetLocalAmbientColour( uiScaled );
	m_pModel->SetAmbientColour( uiScaled );
	m_pModel->UseLocalAmbientLight( TTRUE );
}

// $deBlob: FUNCTION 007b5840
TBOOL APropHazard::Tile::UpdateTurningOn( TFLOAT a_fDeltaTime )
{
	if ( m_uiFlags & TILEFLAG_TurningOn )
	{
		m_fTime = m_fTime + a_fDeltaTime;

		if ( m_fTime < m_fDuration )
		{
			m_pModel->UseLocalAmbientColour( TTRUE );

			// Flickers: lit for 0.05-0.2 s, dark for 0.02-0.05 s
			m_fFlickerTime = m_fFlickerTime - a_fDeltaTime;

			if ( m_fFlickerTime <= 0.0f )
			{
				m_uiFlags ^= TILEFLAG_Lit;

				if ( m_uiFlags & TILEFLAG_Lit )
				{
					m_fFlickerTime = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( 0.05f, 0.2f );
					return TFALSE;
				}

				m_fFlickerTime = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( 0.02f, 0.05f );
			}

			return TFALSE;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 007b58e0
TFLOAT APropHazard::Tile::StartTurningOn()
{
	m_uiFlags |= TILEFLAG_TurningOn;
	m_fTime        = 0.0f;
	m_fFlickerTime = 0.0f;
	m_fDuration    = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( s_fTransitionTime * 0.5f, s_fTransitionTime * 1.5f );

	return m_fDuration;
}

// $deBlob: FUNCTION 007b5990
void APropHazard::Tile::TurnOn()
{
	m_uiFlags      = ( m_uiFlags & ~TILEFLAG_TurningOn ) | TILEFLAG_Lit;
	m_fEffectDelay = ARandom::GetSingleton()->GetRandom().GetFloatMinMax( 0.0f, 0.1f );
}

// $deBlob: FUNCTION 007b5a00
void APropHazard::Tile::TurnOff()
{
	m_uiFlags &= ~( TILEFLAG_TurningOn | TILEFLAG_Lit );

	if ( m_pEffect != TNULL )
	{
		m_pEffect->StopInstant();
		m_pEffect = TNULL;
	}
}

// $deBlob: FUNCTION 007b5b50
void* APropHazard::GetDespawnEffect()
{
	return sm_despawnEffect;
}
