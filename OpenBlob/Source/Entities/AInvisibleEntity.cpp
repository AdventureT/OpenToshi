#include "pch.h"
#include "AInvisibleEntity.h"
#include "AEntityFactory.h"
#include "Collision/ACollisionObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007c6820
// $deBlob: FUNCTION 007c6830
// $deBlob: FUNCTION 007c6840
// $deBlob: FUNCTION 007c6850
// $deBlob: FUNCTION 007c6860
// $deBlob: FUNCTION 00541830
// $deBlob: FUNCTION 0085a080
TDEFINE_CLASS_NORUNTIME( AInvisibleEntity );

// $deBlob: FUNCTION 007c6870
AInvisibleEntity::AInvisibleEntity()
{
	m_pCollisionObject = new ACollisionObject( this, 1, 0 );
	m_pCollisionObject->SetFlags( ACollisionObject::FLAG_Created );
}

// $deBlob: FUNCTION 007c68f0
// $deBlob: FUNCTION 007c6b20
AInvisibleEntity::~AInvisibleEntity()
{
	if ( m_pCollisionObject )
		delete m_pCollisionObject;
}

// $deBlob: FUNCTION 007c6960
void AInvisibleEntity::SetBBox( TVector4 a_vBBox )
{
	ABaseEntity::SetBBox( a_vBBox );
	GetCollisionObject()->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
}

// $deBlob: FUNCTION 007c6990
void AInvisibleEntity::SetPosition( const TVector4& a_rPosition )
{
	m_Flags |= FLAG_UpdateMatrix;
	m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rPosition;

	// NOTE: retail calls it and ignores the result
	GetCollisionObject();
}

// $deBlob: FUNCTION 007c69b0
void AInvisibleEntity::SetMatrix( const TMatrix44& a_rMatrix )
{
	m_oMatrix = a_rMatrix;
	m_Flags |= FLAG_UpdateMatrix;
	GetCollisionObject()->SetTransformDirty( a_rMatrix );
}

// $deBlob: FUNCTION 007c6a80
void AInvisibleEntity::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_DESPAWN && !( TUINT8( m_Flags >> 24 ) & 1 ) )
		Despawn();
}

// $deBlob: FUNCTION 007c6aa0
void AInvisibleEntity::Despawn()
{
	if ( !( TUINT8( m_Flags >> 24 ) & 1 ) )
	{
		RemoveFromPhysics();
		SetVisible( TFALSE );
		SetTicking( TFALSE );
		m_Flags |= 0x1000000 | FLAG_DontTick;
		AEntityFactory::GetSingleton()->DeactivateEntity( *this );
	}
}

// $deBlob: FUNCTION 007c6ae0
void AInvisibleEntity::Reset( ABaseEntity* a_pEntity )
{
	m_Flags = ( m_Flags & ~0x1C00 ) | FLAG_LOD0;

	// NOTE: retail tests the flag in the new value (the mask keeps bit 24)
	if ( TUINT8( m_Flags >> 24 ) & 1 )
	{
		AddToPhysics();
		SetVisible( TTRUE );
		SetTicking( TFALSE );
		m_Flags &= ~( 0x1000000 | FLAG_DontTick );
	}
}
