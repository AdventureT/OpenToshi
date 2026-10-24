#include "pch.h"
#include "AIOCollisionComponent.h"
#include "IO/AIOEntity.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AIOCollisionComponent ) == 0xD0 );

// The translation of a collision's matrix (a copy of TVector4::VEC_ZERO made at start up)
// Dynamic initializer (copies TVector4::VEC_ZERO)
// $deBlob: FUNCTION 0053ba30
static TVector4 s_vCollisionTranslation = TVector4::VEC_ZERO;

// $deBlob: FUNCTION 007cd280
AIOCollisionComponent::AIOCollisionComponent( AIOEntity* a_pOwner )
    : AIOEntityComponent( a_pOwner, TYPE )
    , m_oLocalMatrix( TMatrix44::IDENTITY )
    , m_pCollisionObject( TNULL )
    , m_pModelInstance( TNULL )
    , m_oMover( a_pOwner )
{
	m_uiFlags |= 1;
	m_fPush = 0.0f;
}

// $deBlob: FUNCTION 007cd920
// $deBlob: FUNCTION 007cd330
AIOCollisionComponent::~AIOCollisionComponent()
{
	if ( m_pCollisionObject != TNULL )
		delete m_pCollisionObject;

	if ( m_pModelInstance != TNULL )
		AModelFactory::GetSingleton()->DestroyInstance( m_pModelInstance );
}

// $deBlob: FUNCTION 007cd3c0
void AIOCollisionComponent::SetIOEntityData( void* a_pData, TTRB* a_pTRB )
{
	IOEntityData* pIOData = TSTATICCAST( IOEntityData, a_pData );

	TBOOL bCorrectType = pIOData->iType == TYPE;
	TASSERT( TTRUE == bCorrectType );

	Data* pData        = pIOData->pData;
	m_iUnk14           = pData->iUnk00;
	m_iUnk18           = pData->iUnk04;
	m_uiCollisionFlags = pData->uiFlags;
	m_pPosition        = pData->pPosition;
	m_pRotation        = pData->pRotation;
	m_pUnk28           = pData->pUnk14;
	m_fUnk2C           = pData->fPush;
	m_fPush            = pData->fPush;

	TQuaternion quatRotation( m_pRotation->x, m_pRotation->y, m_pRotation->z, m_pRotation->w );
	m_oLocalMatrix.SetFromQuaternion( quatRotation );

	TVector4 vPosition = *m_pPosition;

	m_pCollisionObject = new ACollisionObject( m_pOwner, 1, 0 );
	m_pCollisionObject->SetFlags( ACollisionObject::FLAG_Created );
	m_pCollisionObject->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
	m_pCollisionObject->SetTransformDirty( m_oLocalMatrix );

	if ( m_uiCollisionFlags & FLAGS_Enabled )
		m_pCollisionObject->SetGroupMask( COLLISION_MASK );

	if ( m_uiCollisionFlags & FLAGS_Unk8 )
		m_pOwner->SetFlags( m_pOwner->GetFlags() | 0x100000 );

	// Z up to Y up
	m_oLocalMatrix.m_f41 = vPosition.x;
	m_oLocalMatrix.m_f42 = -vPosition.z;
	m_oLocalMatrix.m_f43 = vPosition.y;
	m_oLocalMatrix.m_f44 = vPosition.w;
}

// $deBlob: FUNCTION 007cd560
TBOOL AIOCollisionComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	TINT* pParams = *TREINTERPRETCAST( TINT**, a_pMessage + 2 );

	if ( a_pMessage[ 0 ] == ACTION_EnableCollision )
	{
		m_pCollisionObject->SetGroupMask( ( pParams[ 0 ] == 1 ) ? COLLISION_MASK : 0 );
		return TTRUE;
	}

	if ( a_pMessage[ 0 ] == ACTION_SetPush )
		m_fPush = *TREINTERPRETCAST( TFLOAT*, pParams );

	return TFALSE;
}

// $deBlob: FUNCTION 007cd5b0
void AIOCollisionComponent::Tick( TFLOAT a_fDeltaTime )
{
	m_oMover.Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007cd5d0
void AIOCollisionComponent::UpdateWorldTransform( const TMatrix44& a_rEntityMatrix )
{
	if ( m_pCollisionObject != TNULL )
	{
		TMatrix44 matWorld;
		matWorld.Multiply( a_rEntityMatrix, m_oLocalMatrix );

		if ( m_pModelInstance != TNULL )
			m_pModelInstance->SetTransform( matWorld );

		matWorld.m_f41 = 0.0f;
		matWorld.m_f42 = 0.0f;
		matWorld.m_f43 = 0.0f;
		m_pCollisionObject->SetTransformDirty( matWorld );
	}
}

// $deBlob: FUNCTION 007cd670
void AIOCollisionComponent::GetMatrix( TMatrix44& a_rOutMatrix )
{
	if ( m_pCollisionObject != TNULL )
	{
		a_rOutMatrix                                           = TMatrix44::IDENTITY;
		a_rOutMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = s_vCollisionTranslation;
		return;
	}

	a_rOutMatrix = m_oLocalMatrix;
}

// $deBlob: FUNCTION 007cd700
void AIOCollisionComponent::SetCollisionDirty( const TVector4& a_rUnused )
{
	if ( m_pCollisionObject != TNULL )
		m_pCollisionObject->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
}

// $deBlob: FUNCTION 007cd710
void AIOCollisionComponent::GetScale( TVector4& a_rOutScale )
{
	// NOTE: both cases give 1
	if ( m_pCollisionObject != TNULL )
	{
		a_rOutScale = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
		return;
	}

	a_rOutScale = TVector4( 1.0f, 1.0f, 1.0f, 1.0f );
}

// $deBlob: FUNCTION 007cd750
void AIOCollisionComponent::GetPosition( TVector4& a_rOutPosition )
{
	if ( m_pCollisionObject != TNULL )
	{
		a_rOutPosition = s_vCollisionTranslation;
		return;
	}

	a_rOutPosition = TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 007cd780
void AIOCollisionComponent::GetRotationMatrix( TMatrix44& a_rOutMatrix )
{
	if ( m_pCollisionObject != TNULL )
	{
		a_rOutMatrix = TMatrix44::IDENTITY;
		return;
	}

	TMatrix44 matLocal;
	GetMatrix( matLocal );

	a_rOutMatrix       = matLocal;
	a_rOutMatrix.m_f41 = 0.0f;
	a_rOutMatrix.m_f42 = 0.0f;
	a_rOutMatrix.m_f43 = 0.0f;
}

// $deBlob: FUNCTION 007cd820
TVector4 AIOCollisionComponent::GetReactionForce( const TVector4& a_rvPoint, ABaseEntity* a_pEntity )
{
	if ( TUINT8( a_pEntity->GetCollisionGroupAndType() ) & m_pCollisionObject->GetGroupMask() )
	{
		TVector4 vForce = m_oMover.GetReactionForce( a_rvPoint, a_pEntity );

		if ( m_fPush != 0.0f )
		{
			TVector4 vPush;
			vPush.Multiply( m_pOwner->GetMatrix().AsBasisVector4( BASISVECTOR_RIGHT ), m_fPush );

			vForce.x += vPush.x;
			vForce.y += vPush.y;
			vForce.z += vPush.z;
		}

		return vForce;
	}

	return TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 007cd8d0
TQuaternion AIOCollisionComponent::GetReactionRotation( ABaseEntity* a_pEntity )
{
	if ( TUINT8( a_pEntity->GetCollisionGroupAndType() ) & m_pCollisionObject->GetGroupMask() )
		return m_oMover.GetReactionRotation( a_pEntity );

	return TQuaternion( 0.0f, 0.0f, 0.0f, 1.0f );
}
