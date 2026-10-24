#include "pch.h"
#include "AProp.h"
#include "AEntityParams.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionObject.h"
#include "Collision/ACollisionInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobColourComponent.h"
#include "ABlobEntity/ABlobColourMaps.h"

#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>
#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00774380
// $deBlob: FUNCTION 00774390
// $deBlob: FUNCTION 00774440
// $deBlob: FUNCTION 007744f0
// $deBlob: FUNCTION 00774500
// $deBlob: FUNCTION 00540960
// $deBlob: FUNCTION 00859cd0
TDEFINE_CLASS( AProp );

// $deBlob: FUNCTION 00774510
AProp::AProp()
{
	m_iManualAnim      = -1;
	m_pCollisionObject = TNULL;
	m_uiPropFlags      = 0;
}

// $deBlob: FUNCTION 00774590
// $deBlob: FUNCTION 00774e10
// $deBlob: FUNCTION 00774df9
AProp::~AProp()
{
	if ( m_pCollisionObject )
		delete m_pCollisionObject;
}

// $deBlob: FUNCTION 00774610
void AProp::Create()
{
	ABaseEntity::Create();
	SetVisible( TTRUE );
}

// $deBlob: FUNCTION 00774630
// $deBlob: FUNCTION 00774ee3
void AProp::ProcessParams( const AEntityParams& a_rParams )
{
	AVisibleEntity::ProcessParams( a_rParams );
	ATriggerUtil::ProcessParams( a_rParams );

	const TCHAR* szMesh = TNULL;
	if ( m_pModelComponent == TNULL )
		a_rParams.GetValue( szMesh, "Mesh" );

	TBOOL bPaintable = TFALSE;
	a_rParams.GetValue( bPaintable, "Paintable" );

	if ( bPaintable )
		m_uiPropFlags |= PROPFLAG_Paintable;
	else
		m_uiPropFlags &= ~PROPFLAG_Paintable;

	if ( szMesh != TNULL && *szMesh != '\0' )
	{
		CreateModelComponent( szMesh );
		CreateFXBreakpointComponent( szMesh );
	}

	if ( GetModelComponent() )
		CreateCollisionObject();
}

// $deBlob: FUNCTION 007746d0
// $deBlob: FUNCTION 00774ed8
void AProp::PostCreate()
{
	ATriggerUtil::PostCreate();
}

// $deBlob: FUNCTION 007746e0
void AProp::Tick( TFLOAT a_fDeltaTime )
{
	AVisibleEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00774700
void AProp::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );

	TINT iPacket = a_rPacket.m_eMessage;

	switch ( iPacket )
	{
		case AEntityIOPacket::MESSAGE_TOUCH:
			if ( ( m_uiPropFlags & PROPFLAG_Paintable ) && a_pCaller != TNULL )
			{
				// Painted by Blob
				if ( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) && GetModelComponent() != TNULL )
					ApplyBlobColour( a_pCaller, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance() );
			}
			break;
		case AEntityIOPacket::MESSAGE_RESET:
			Reset( a_pCaller );
			break;
	}
}

// $deBlob: FUNCTION 00774780
void AProp::SetPosition( const TVector4& a_rPosition )
{
	m_Flags |= FLAG_UpdateMatrix;
	m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rPosition;

	// NOTE: retail calls it twice and ignores the result
	if ( GetCollisionObject() )
		GetCollisionObject();
}

// $deBlob: FUNCTION 00774800
void AProp::SetMatrix( const TMatrix44& a_rMatrix )
{
	m_oMatrix = a_rMatrix;
	m_Flags |= FLAG_UpdateMatrix;

	if ( GetCollisionObject() && ( m_pCollisionObject->GetFlags() & ACollisionObject::FLAG_Created ) )
		m_pCollisionObject->SetTransformDirty( a_rMatrix );
}

// $deBlob: FUNCTION 007747b0
void AProp::SetBBox( TVector4 a_vBBox )
{
	ABaseEntity::SetBBox( a_vBBox );

	if ( GetCollisionObject() )
	{
		m_fBoundingSphereRadius = a_vBBox.Magnitude();
		m_pCollisionObject->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
		m_pCollisionObject->SetGroupMask( 0xFB );
	}
}

// $deBlob: FUNCTION 007748f0
TINT16 AProp::CreateManualAnimation( const TCHAR* a_szName )
{
	if ( GetModelComponent() == TNULL )
		return -1;

	return TINT16( TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetAnimID( a_szName ) );
}

// $deBlob: FUNCTION 00774920
TAnimation* AProp::AddManualAnimation( TINT16 a_iSequence, TFLOAT a_fTime, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed )
{
	if ( GetModelComponent() == TNULL )
		return TNULL;

	AToshiAnimInterface2* pAnimInterface = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();
	pAnimInterface->StopAllAnims( 0.15f );

	TAnimation* pAnimation = pAnimInterface->AddAnim( a_iSequence, a_fBlendInSpeed, a_fWeight, a_fSpeed );

	if ( pAnimation == TNULL )
		return TNULL;

	if ( a_fTime == -1.0f )
		a_fTime = pAnimation->GetSequencePtr()->GetDuration();

	pAnimation->SetTime( a_fTime );
	pAnimation->SetState( TAnimation::STATE_BLENDING_IN );
	m_iManualAnim = a_iSequence;

	return pAnimation;
}

// $deBlob: FUNCTION 007749e0
void AProp::KillManualAllAnimations( TFLOAT a_fBlendOutSpeed )
{
	if ( GetModelComponent() == TNULL )
		return;

	AToshiAnimInterface2* pAnimInterface = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();

	if ( a_fBlendOutSpeed != -1.0f )
		pAnimInterface->StopAllAnims( a_fBlendOutSpeed );
	else
		pAnimInterface->StopAllAnims( 0.15f );
}

// $deBlob: FUNCTION 00774a30
TAnimation* AProp::GetPlayingManualAnimation()
{
	if ( GetModelComponent() != TNULL )
	{
		AToshiAnimInterface2* pAnimInterface = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();

		if ( pAnimInterface->GetSkeletonInstance() != TNULL )
		{
			TAnimation* pAnimation = pAnimInterface->GetSkeletonInstance()->IsAnimating( m_iManualAnim );

			if ( pAnimation != TNULL && 0.0f < pAnimation->GetTotalTime() && pAnimation->GetTotalTime() < pAnimation->GetSequencePtr()->GetDuration() )
				return pAnimation;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00774aa0
TBOOL AProp::IsManualAnimationFinished( TINT16 a_iSequence )
{
	if ( GetModelComponent() == TNULL )
		return TFALSE;

	AToshiAnimInterface2* pAnimInterface = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();

	if ( pAnimInterface->GetSkeletonInstance() != TNULL )
	{
		TAnimation* pAnimation = pAnimInterface->GetSkeletonInstance()->IsAnimating( a_iSequence );

		if ( pAnimation != TNULL && pAnimation->GetSequencePtr()->GetDuration() <= pAnimation->GetTotalTime() )
		{
			pAnimInterface->StopAnim( a_iSequence, 0.15f );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00774b20
void AProp::Reset( ABaseEntity* a_pEntity )
{
	if ( GetModelComponent() )
	{
		AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();

		if ( pModelInstance && ( m_uiPropFlags & PROPFLAG_Paintable ) )
		{
			pModelInstance->SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
			pModelInstance->UseLocalAmbientLight( TFALSE );
		}
	}

	AVisibleEntity::Reset( a_pEntity );
}

// $deBlob: FUNCTION 00774b70
TBOOL AProp::IsPainted()
{
	if ( GetModelComponent() != TNULL )
		return TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetLocalAmbientColour() != AModelInstance2::ms_uiDefaultLocalAmbientColour;

	return TFALSE;
}

// $deBlob: FUNCTION 00774ba0
void AProp::CreateCollisionObject()
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

// $deBlob: FUNCTION 00774c30
void AProp::CreateModelCollision( TBOOL a_bPrimitives )
{
	TModelInstance* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();
	T2ModelPtr      pModel         = pModelInstance->GetModel();

	ACollisionInstance2* pCollision = new ACollisionInstance2();
	pCollision->SetModelInstance( pModelInstance, TNULL );

	ACollisionInstance2::OwnerInfo oOwner;
	oOwner.m_eType  = 2;
	oOwner.m_pOwner = this;
	pCollision->Create( pModelInstance, &oOwner, a_bPrimitives, 0, TNULL );

	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetCollisionInstance( pCollision );
}

// Takes Blob's (a_pCaller's) colour on a_pModelInstance (it costs a paint point), ink takes the colour off
// $deBlob: FUNCTION 00774d00
void AProp::ApplyBlobColour( ABaseEntity* a_pCaller, void* a_pModelInstance )
{
	ABlobEntity*     pBlob          = TSTATICCAST( ABlobEntity, a_pCaller );
	AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, a_pModelInstance );
	TUINT32          uiColour       = pBlob->GetColourComponent()->GetCurrentColour();

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
		uiColour = ABlobColourComponent::COLOUR_BLACK;
	else if ( uiColour == 0 )
		return;

	if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
	{
		pModelInstance->SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
		pModelInstance->UseLocalAmbientLight( TFALSE );
		return;
	}

	TUINT32 uiBodyColour = ABlobColourMaps::ms_pEnemyBodyColourMap[ uiColour ];

	if ( pModelInstance->GetLocalAmbientColour() != uiBodyColour )
	{
		pModelInstance->SetLocalAmbientColour( uiBodyColour );
		pModelInstance->UseLocalAmbientColour( TTRUE );
		pModelInstance->UseLocalAmbientLight( TTRUE );
		pModelInstance->SetAmbientColour( ABlobColourMaps::ms_pEnemyLightColourMap[ uiColour ] );
		pBlob->RemovePaintPoints( 1, TFALSE );
	}
}

// Replaces the model ("Mesh") and its collision
// $deBlob: FUNCTION 00774dc0
void AProp::SetMesh( const TCHAR* a_szMesh )
{
	if ( a_szMesh != TNULL && *a_szMesh != '\0' )
	{
		CreateModelComponent( a_szMesh );
		CreateFXBreakpointComponent( a_szMesh );
	}

	if ( GetModelComponent() != TNULL )
		CreateCollisionObject();
}

// $deBlob: FUNCTION 00774da0
void AProp::ResetAmbientColour( AModelInstance2* a_pModelInstance )
{
	a_pModelInstance->SetLocalAmbientColour( AModelInstance2::ms_uiDefaultLocalAmbientColour );
	a_pModelInstance->UseLocalAmbientLight( TFALSE );
}
