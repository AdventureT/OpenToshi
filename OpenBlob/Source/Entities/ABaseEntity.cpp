#include "pch.h"
#include "ABaseEntity.h"
#include "AEntityFactory.h"
#include "AEntityHash.h"
#include "AEntityParams.h"
#include "AEntityStateComponent.h"
#include "AEntityPoseController.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ABaseEntity* ABaseEntity::sm_EntityHash[ ENTITY_HASH_SIZE ];
TINT         ABaseEntity::ms_iNextPooledID = ENTITY_HASH_SIZE;

// NOTE: retail's CreateObject/CreateObjectInPlace return TNULL without the NORUNTIME assert
// $deBlob: FUNCTION 0075e530
// $deBlob: FUNCTION 0075e540
// $deBlob: FUNCTION 0075e550
// $deBlob: FUNCTION 0075e560
// $deBlob: FUNCTION 0075e570
// $deBlob: FUNCTION 005407a0
// $deBlob: FUNCTION 00859c50
TDEFINE_CLASS_NORUNTIME( ABaseEntity );

TSTATICASSERT( sizeof( ABaseEntity ) == 0x80 );

// Dynamic initializer (atexit) and its atexit destructor
// $deBlob: FUNCTION 005407e0
// $deBlob: FUNCTION 00859c60
ABaseEntity::NameMap ABaseEntity::sm_oNameMap;

// $deBlob: FUNCTION 0075e580
ABaseEntity::ABaseEntity()
{
	m_iState                 = -1;
	m_iID                    = -1;
	m_oMatrix                = TMatrix44::IDENTITY;
	m_vBBox                  = TVector4::VEC_ZERO;
	m_iLife                  = 0;
	m_iCollisionGroupAndType = -1;
	m_uiHashTimeStamp        = 0;
	m_fBoundingSphereRadius  = 1.0f;
	m_fStateTime             = 0.0f;
	m_Flags                  = FLAG_PhysicsEnabled | FLAG_UpdateMatrix | FLAG_LOD0 | FLAG_Hashed;
	m_pTypeData              = TNULL;
	m_pHashRef               = TNULL;
	m_pEntityCellPair        = TNULL;

	AEntityFactory::GetSingleton()->IncrementEntityFlagsIndex();
	m_pEntityCellPair = new ACellPair( this );
}

// $deBlob: FUNCTION 0075e6f0
ABaseEntity::~ABaseEntity()
{
	AEntityHash::GetSingleton()->Remove( *this );

	if ( !IsPooled() && m_iID >= 0 )
	{
		TASSERT( m_iID < ENTITY_HASH_SIZE );
		TASSERT( sm_EntityHash[ m_iID ] == this );
		sm_EntityHash[ m_iID ] = TNULL;

		for ( auto it = sm_oNameMap.Begin(); it != sm_oNameMap.End(); )
		{
			auto itCur = it++;

			if ( itCur->GetSecond() == m_iID )
				sm_oNameMap.Remove( itCur );
		}
	}

	if ( m_pEntityCellPair )
		delete m_pEntityCellPair;
}

// $deBlob: FUNCTION 0075e8e0
void ABaseEntity::SetTicking( TBOOL a_bTicking )
{
	if ( a_bTicking )
	{
		if ( !( TUINT8( m_Flags >> 8 ) & 1 ) )
		{
			TASSERT( !IsFlagged( FLAG_DontTick ) );
			m_Flags |= FLAG_Ticking;
			AEntityFactory::GetSingleton()->MarkStale( *this );
		}
	}
	else if ( TUINT8( m_Flags >> 8 ) & 1 )
	{
		m_Flags &= ~FLAG_Ticking;
		AEntityFactory::GetSingleton()->MarkStale( *this );
	}
}

// $deBlob: FUNCTION 0075e970
void ABaseEntity::Create()
{
	TASSERT( 0 == ( m_Flags & FLAG_Created ) );
	m_Flags |= FLAG_Created;
}

// $deBlob: FUNCTION 0075e9b0
void ABaseEntity::Destroy()
{
	TASSERT( 0 != ( m_Flags & FLAG_Created ) );
	TASSERT( !m_pEntityCellPair->IsLinked() );
	m_pEntityCellPair->m_pCell = TNULL;
}

// $deBlob: FUNCTION 0075ea30
void ABaseEntity::ProcessParams( const AEntityParams& a_rParams )
{
	TBOOL bIgnoreCellForLOD = TFALSE;
	if ( a_rParams.GetValue( bIgnoreCellForLOD, "IgnoreCellForLOD" ) )
	{
		if ( bIgnoreCellForLOD )
			m_Flags |= FLAG_IgnoreCellForLOD;
		else
			m_Flags &= ~FLAG_IgnoreCellForLOD;
	}

	TBOOL bUseLargeLODDistance = TFALSE;
	if ( a_rParams.GetValue( bUseLargeLODDistance, "UseLargeLODDistance" ) )
	{
		if ( bUseLargeLODDistance )
			m_Flags |= FLAG_UseLargeLODDistance;
		else
			m_Flags &= ~FLAG_UseLargeLODDistance;
	}
}

// $deBlob: FUNCTION 0075eab0
void ABaseEntity::SetState( TINT a_State )
{
	TASSERT( a_State >= 0 );

	if ( a_State != m_iState )
	{
		OnLeaveState( m_iState );
		m_iState     = TINT16( a_State );
		m_fStateTime = 0.0f;
		OnEnterState( TINT16( a_State ) );
	}
}

// $deBlob: FUNCTION 0075eb20
void ABaseEntity::SetHash( TBOOL a_bHashed )
{
	if ( !a_bHashed )
	{
		m_Flags &= ~FLAG_Hashed;
		AEntityHash::GetSingleton()->Remove( *this );
	}
	else
	{
		if ( !( TUINT8( m_Flags >> 1 ) & 1 ) )
		{
			m_Flags |= FLAG_Hashed;
			AEntityHash::GetSingleton()->Add( *this, TFALSE );
		}
	}
}

// $deBlob: FUNCTION 0075eb80
TINT ABaseEntity::GetIDName( const TCHAR* a_szName )
{
	auto it = sm_oNameMap.Find( TString8( a_szName ) );

	if ( it == sm_oNameMap.End() )
		return -1;

	return it->GetSecond();
}

// $deBlob: FUNCTION 0075ec20
void ABaseEntity::SetName( const TCHAR* a_szName )
{
	TASSERT( !IsPooled() );

	TString8 strName( a_szName );

	if ( strName.Length() > 0 )
		sm_oNameMap.Insert( strName, m_iID );
}

// $deBlob: FUNCTION 0075ece0
TBOOL ABaseEntity::HasStateFinished()
{
	if ( 0.0f < TSTATICCAST( AEntityStateComponent, GetStateComponent() )->GetStateLength( m_iState ) )
		return TSTATICCAST( AEntityStateComponent, GetStateComponent() )->GetStateLength( m_iState ) <= m_fStateTime;

	TINT iState = m_iState;

	if ( TSTATICCAST( AEntityPoseController, GetPoseController() )->GetCurrentState() == iState )
	{
		if ( TSTATICCAST( AEntityPoseController, GetPoseController() )->GetPoseLength() <= m_fStateTime )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0075ed70
TBOOL ABaseEntity::HasStateTimedOut()
{
	return TSTATICCAST( AEntityStateComponent, GetStateComponent() )->GetStateLength( m_iState ) <= m_fStateTime;
}

// $deBlob: FUNCTION 0075eda0
TBOOL ABaseEntity::HasPoseFinished()
{
	TINT iState = m_iState;

	if ( TSTATICCAST( AEntityPoseController, GetPoseController() )->GetCurrentState() == iState )
	{
		if ( TSTATICCAST( AEntityPoseController, GetPoseController() )->GetPoseLength() <= m_fStateTime )
			return TTRUE;
	}

	return TFALSE;
}

TVector4 ABaseEntity::GetBasePosition() const
{
	const TVector4& rUp  = m_oMatrix.AsBasisVector4( BASISVECTOR_UP );
	const TVector4& rPos = m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	TFLOAT          fH   = -m_vBBox.y;

	return TVector4( rUp.x * fH + rPos.x, rUp.y * fH + rPos.y, rUp.z * fH + rPos.z, rPos.w );
}
