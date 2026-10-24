#include "pch.h"
#include "AFXBreakpointComponent.h"
#include "Entities/AEntityData.h"
#include "Entities/ACharEntity.h"
#include "Entities/AGndMotorComponent.h"
#include <Physics/TSurfaceType.h>
#include "Entities/AEntityModelComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "FXComposition/AFXCompositionInstance.h"
#include "Entities/AFXHitEntityCollisionFunctor.h"
#include "Collision/ACollisionManager2.h"
#include <Render/TAnimation.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0076b250
// $deBlob: FUNCTION 0076c110
AFXBreakpointComponent::~AFXBreakpointComponent()
{
	if ( m_pCompositions != TNULL )
		delete[] m_pCompositions;

	m_pCompositions = TNULL;
}

// $deBlob: FUNCTION 0076b2e0
AFXBreakpointComponent* AFXBreakpointComponent::Create( const TCHAR* a_szModelName, TMemoryHeap* a_pHeap, ABaseEntity* a_pOwner, AModelInstance2* a_pModelInstance )
{
	Data* pData = TSTATICCAST( Data, AEntityData::GetSingleton()->GetFXBreakpointData( "Main" ) );

	if ( pData == TNULL )
		return TNULL;

	const TCHAR* szName = a_szModelName;

	for ( const TCHAR* pSlash = TStringManager::String8FindChar( szName, '/' ); pSlash != TNULL; pSlash = TStringManager::String8FindChar( szName, '/' ) )
		szName = pSlash + 1;

	for ( const TCHAR* pSlash = TStringManager::String8FindChar( szName, '\\' ); pSlash != TNULL; pSlash = TStringManager::String8FindChar( szName, '\\' ) )
		szName = pSlash + 1;

	TString8 strName( szName );

	for ( TINT i = 0; i < pData->m_iNumModels; i++ )
	{
		if ( strName.CompareNoCase( pData->m_pModels[ i ].m_szModel ) != 0 )
			continue;

		Groups* pGroups = &pData->m_pModels[ i ];

		AModelInstance2* pModelInstance = a_pModelInstance;

		if ( a_pOwner != TNULL )
		{
			if ( ( a_pOwner->GetFlags() & 0x10 ) == 0 )
			{
				pModelInstance = TSTATICCAST( AEntityModelComponent, a_pOwner->GetModelComponent() )->GetModelInstance();
			}
			else
			{
				TASSERT( a_pOwner->IsA( &TGetClass( ABlobEntity ) ) );
				pModelInstance = TSTATICCAST( ABlobEntity, a_pOwner )->GetAnimationComponent()->GetCharModelInstance();
			}
		}

		AToshiAnimInterface2* pAnimInterface = pModelInstance->GetAnimInterface();
		TUINT                 uiNumFX        = 0;

		for ( TINT j = 0; j < pGroups->m_iNumGroups; j++ )
		{
			pGroups->groups[ j ].animationID = TINT16( pAnimInterface->GetAnimID( pGroups->groups[ j ].m_szAnimation ) );
			TASSERT( pGroups->groups[ j ].animationID >= 0 );
			uiNumFX += pGroups->groups[ j ].m_iNumFX;
		}

		if ( uiNumFX == 0 )
			return TNULL;

		AFXBreakpointComponent* pComponent;

		if ( a_pOwner == TNULL )
		{
			pComponent                   = new ( a_pHeap ) AFXBreakpointComponent( TNULL );
			pComponent->m_pModelInstance = a_pModelInstance;
		}
		else
		{
			pComponent = new ( a_pHeap ) AFXBreakpointComponent( a_pOwner );
		}

		pComponent->m_pData            = pData;
		pComponent->m_pGroups          = pGroups;
		pComponent->m_iNumCompositions = uiNumFX;

		pComponent->m_pCompositions = new ( a_pHeap ) AFXCompositionInstance[ uiNumFX ];

		TINT16 iIndex = 0;

		for ( TINT j = 0; j < pGroups->m_iNumGroups; j++ )
		{
			for ( TINT k = 0; k < pGroups->groups[ j ].m_iNumFX; k++ )
			{
				FX* pFX             = &pGroups->groups[ j ].m_pFX[ k ];
				pFX->m_iAnimationID = TINT16( pGroups->groups[ j ].animationID );
				pFX->m_iIndex       = iIndex;
				pComponent->m_pCompositions[ iIndex ].Initialize( pFX->m_szComposition );
				iIndex++;
			}
		}

		return pComponent;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0076b650
void AFXBreakpointComponent::Tick( TFLOAT a_fDeltaTime )
{

	if ( m_iNumCompositions == 0 )
		return;

	for ( TINT i = 0; i < m_iNumCompositions; i++ )
		m_pCompositions[ i ].Update( a_fDeltaTime );

	UpdateBoneAttachedFX();

	ABaseEntity* pOwner       = m_pEntity;
	TINT         iCharVariant = -1;

	if ( pOwner != TNULL )
	{
		if ( pOwner->GetLOD() != ABaseEntity::FLAG_HighLOD )
			return;

		if ( !IsEnabled() )
			return;

		// A character's ground surface picks the effects of that surface
		if ( pOwner->IsA( &TGetClass( ACharEntity ) ) )
			iCharVariant = TSTATICCAST( AGndMotorComponent, TSTATICCAST( ACharEntity, pOwner )->GetMotorComponent() )->GetSurface()->m_iIndex;
	}

	AToshiAnimInterface2* pAnimInterface = GetModelInstance()->GetAnimInterface();

	for ( TINT i = 0; i < m_pGroups->m_iNumGroups; i++ )
	{
		Group* pGroup = &m_pGroups->groups[ i ];

		for ( TINT j = 0; j < pGroup->m_iNumFX; j++ )
		{
			FX*         pFX        = &pGroup->m_pFX[ j ];
			TAnimation* pAnimation = TNULL;
			TBOOL       bTrigger   = TFALSE;

			if ( pAnimInterface->GetSkeletonInstance() == TNULL || ( pAnimation = pAnimInterface->GetSkeletonInstance()->IsAnimating( pFX->m_iAnimationID ) ) == TNULL )
			{
				// The animation isn't playing: stop its bone attached compositions
				for ( TINT k = 0; k < MAX_ATTACHED_FX; k++ )
				{
					FX* pAttachedFX = m_aAttached[ k ].m_pFX;

					if ( pAttachedFX != TNULL && pAttachedFX->m_iAnimationID == pFX->m_iAnimationID && ( pAttachedFX->m_uiFlags & 1 ) )
						m_aAttached[ k ].m_pComposition->Stop();
				}
			}
			else
			{
				if ( pFX->m_iIndex < 0 )
					continue;

				TUINT16 uiFlags   = pFX->m_uiFlags;
				TBOOL   bAttached = TFALSE;

				if ( uiFlags & 1 )
				{
					for ( TINT k = 0; k < MAX_ATTACHED_FX; k++ )
					{
						if ( m_aAttached[ k ].m_pFX == pFX )
						{
							bAttached = TTRUE;
							break;
						}
					}
				}

				// The animation passed the breakpoint's time this frame
				bTrigger = !bAttached && pFX->m_fTime < pAnimation->GetSeqTime() &&
				    pAnimation->GetSeqTime() - pAnimation->GetSpeed() * a_fDeltaTime <= pFX->m_fTime &&
				    ( !( uiFlags & 2 ) || pFX->m_iVariant == -1 ) && ( !( uiFlags & 8 ) || pFX->m_iVariant == iCharVariant );
			}

			if ( !bTrigger )
			{
				for ( TINT k = 0; k < MAX_ACTIVE_FX; k++ )
				{
					if ( m_apActiveFX[ k ] == pFX )
					{
						m_apActiveFX[ k ] = TNULL;
						break;
					}
				}
			}
			else if ( m_apActiveFX[ 0 ] != pFX && m_apActiveFX[ 1 ] != pFX && m_apActiveFX[ 2 ] != pFX && m_apActiveFX[ 3 ] != pFX && m_apActiveFX[ 4 ] != pFX )
			{
				TriggerBreakpoint( pFX );

				for ( TINT k = 0; k < MAX_ACTIVE_FX; k++ )
				{
					if ( m_apActiveFX[ k ] == TNULL )
					{
						m_apActiveFX[ k ] = pFX;
						break;
					}
				}
			}
		}
	}
}

// $deBlob: FUNCTION 0076b9a0
void AFXBreakpointComponent::SetVisible( TBOOL a_bVisible )
{
	m_bEnabled = a_bVisible;

	if ( m_iNumCompositions != 0 && !a_bVisible )
	{
		for ( TINT i = 0; i < m_iNumCompositions; i++ )
			m_pCompositions[ i ].Stop();

		for ( TINT i = 0; i < MAX_ACTIVE_FX; i++ )
			m_apActiveFX[ i ] = TNULL;

		for ( TINT i = 0; i < MAX_ATTACHED_FX; i++ )
		{
			m_aAttached[ i ].m_pFX          = TNULL;
			m_aAttached[ i ].m_pComposition = TNULL;
		}
	}
}

// $deBlob: FUNCTION 0076ba30
void AFXBreakpointComponent::ResetComponent()
{
	for ( TINT i = 0; i < m_iNumCompositions; i++ )
		m_pCompositions[ i ].StopInstant();

	for ( TINT i = 0; i < MAX_ACTIVE_FX; i++ )
		m_apActiveFX[ i ] = TNULL;

	for ( TINT i = 0; i < MAX_ATTACHED_FX; i++ )
	{
		m_aAttached[ i ].m_pFX          = TNULL;
		m_aAttached[ i ].m_pComposition = TNULL;
	}
}

// $deBlob: FUNCTION 0076bab0
AFXBreakpointComponent::AFXBreakpointComponent( ABaseEntity* a_pOwner )
    : AEntityComponent2( a_pOwner )
{
	m_pData         = TNULL;
	m_pCompositions = TNULL;

	for ( TINT i = 0; i < MAX_ATTACHED_FX; i++ )
	{
		m_aAttached[ i ].m_pFX          = TNULL;
		m_aAttached[ i ].m_pComposition = TNULL;
	}

	m_iNumCompositions = 0;
	m_pGroups          = TNULL;
	m_pModelInstance   = TNULL;

	for ( TINT i = 0; i < MAX_ACTIVE_FX; i++ )
		m_apActiveFX[ i ] = TNULL;
}

// $deBlob: FUNCTION 0076bb40
AModelInstance2* AFXBreakpointComponent::GetModelInstance()
{
	ABaseEntity* pOwner = m_pEntity;

	if ( pOwner == TNULL )
		return m_pModelInstance;

	if ( TUINT8( pOwner->GetFlags() >> 4 ) & 1 )
	{
		TASSERT( pOwner->IsA( &TGetClass( ABlobEntity ) ) );
		return TSTATICCAST( ABlobEntity, pOwner )->GetAnimationComponent()->GetCharModelInstance();
	}

	return TSTATICCAST( AEntityModelComponent, pOwner->GetModelComponent() )->GetModelInstance();
}

// $deBlob: FUNCTION 0076bbc0
void AFXBreakpointComponent::UpdateBoneAttachedFX()
{
	for ( TINT i = 0; i < MAX_ATTACHED_FX; i++ )
	{
		AttachedFX& rAttached = m_aAttached[ i ];

		if ( rAttached.m_pFX == TNULL )
			continue;

		if ( !rAttached.m_pComposition->IsFinished() )
		{
			TMatrix44 mBone;
			GetModelInstance()->GetBoneTransform( rAttached.m_pFX->m_iBone, mBone );

			if ( rAttached.m_pFX->m_uiFlags & 4 )
			{
				TMatrix44 mTransform                                 = TMatrix44::IDENTITY;
				mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
				rAttached.m_pComposition->SetTransform( mTransform );
			}
			else
			{
				rAttached.m_pComposition->SetTransform( mBone );
			}
		}
		else
		{
			rAttached.m_pFX          = TNULL;
			rAttached.m_pComposition = TNULL;
		}
	}
}

// $deBlob: FUNCTION 0076bc90
void AFXBreakpointComponent::TriggerBreakpoint( FX* a_pFX )
{
	TINT iBone = a_pFX->m_iBone;

	if ( iBone < 0 )
	{
		TMatrix44 mTransform;

		if ( m_pEntity == TNULL )
			m_pModelInstance->GetTransform( mTransform );
		else
			mTransform = m_pEntity->GetMatrix();

		if ( a_pFX->m_uiFlags & 0x10 )
			ComputeDecalMatrix( mTransform );

		m_pCompositions[ a_pFX->m_iIndex ].TriggerEffect( mTransform );
		return;
	}

	TMatrix44 mBone;
	GetModelInstance()->GetBoneTransform( iBone, mBone );

	if ( a_pFX->m_uiFlags & 4 )
	{
		TMatrix44 mTransform                                 = TMatrix44::IDENTITY;
		mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
		m_pCompositions[ a_pFX->m_iIndex ].TriggerEffect( mTransform );
	}
	else
	{
		if ( a_pFX->m_uiFlags & 0x10 )
			ComputeDecalMatrix( mBone );

		m_pCompositions[ a_pFX->m_iIndex ].TriggerEffect( mBone );
	}

	TINT  iEmptyFXIndex    = -1;
	TBOOL bAlreadyAttached = TFALSE;

	for ( TINT i = 0; i < MAX_ATTACHED_FX; i++ )
	{
		if ( a_pFX == m_aAttached[ i ].m_pFX )
			bAlreadyAttached = TTRUE;
		else if ( m_aAttached[ i ].m_pFX == TNULL )
			iEmptyFXIndex = i;
	}

	TASSERT( bAlreadyAttached || 0 <= iEmptyFXIndex, "Ran out of free Bone Attached FX slots, increase MAX_BONE_ATTACHED_FX" );

	if ( bAlreadyAttached || iEmptyFXIndex < 0 )
		return;

	m_aAttached[ iEmptyFXIndex ].m_pFX          = a_pFX;
	m_aAttached[ iEmptyFXIndex ].m_pComposition = &m_pCompositions[ a_pFX->m_iIndex ];
}

// $deBlob: FUNCTION 0076be30
void AFXBreakpointComponent::ComputeDecalMatrix( TMatrix44& a_rMatrix )
{
	static const TVector4 s_vDown( 0.0f, 1.0f, 0.0f, 1.0f );

	AFXHitEntityCollisionFunctor oFunctor;
	TVector4                     vPosition = a_rMatrix.GetTranslation();
	TFLOAT                       fDistance = 6.0f;

	// NOTE: retail asks for the collision object twice (this first result isn't used)
	if ( m_pEntity != TNULL )
		m_pEntity->GetCollisionObject();

	// A ray down from just above the position
	TVector4 vOffset;
	vOffset.Multiply( TVector4::VEC_NEGY, 0.1f );
	TVector4 vStart( vOffset.x + vPosition.x, vOffset.y + vPosition.y, vOffset.z + vPosition.z, vPosition.w );

	TVector4          vNormal;
	ACollisionObject* pHit = ACollisionManager2::CollideRay( vStart, s_vDown, fDistance, vNormal, m_pEntity->GetCollisionObject(), &oFunctor, 0x87, TNULL );

	if ( pHit == TNULL )
	{
		a_rMatrix.Identity();
	}
	else
	{
		// Face the terrain's normal (at most 40 degrees from up), up for anything else
		TVector4 vDirection = TVector4::VEC_NEGY;

		if ( HASANYFLAG( pHit->GetFlags(), ACollisionObject::FLAG_OwnerType0 ) )
			vDirection = vNormal;

		vDirection.ClampLengthXZ( TFLOAT( tan( 0.6981317400932312 ) ) * vDirection.y );
		vDirection.Normalise();
		a_rMatrix.LookAtDirection( vDirection, TVector4::VEC_NEGY );
		a_rMatrix.RotateX( -1.5707964f );
	}

	TASSERT( vPosition.w == 1.0f );
	a_rMatrix.SetTranslation( vPosition );
}
