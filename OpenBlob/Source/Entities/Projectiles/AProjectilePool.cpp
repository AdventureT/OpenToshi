#include "pch.h"
#include "AProjectilePool.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007d5cf0
AProjectilePool::AProjectilePool( AProjectile** a_ppProjectiles, TUINT a_uiSize )
    : m_ppProjectiles( a_ppProjectiles )
    , m_uiSize( a_uiSize )
{
	TASSERT( a_uiSize > 0 );
}

// $deBlob: FUNCTION 007d5ef0
AProjectilePool::~AProjectilePool()
{
	KillAll();
}

// $deBlob: FUNCTION 007d5e90
AProjectile* AProjectilePool::GetFreeProjectile()
{
	TASSERT( m_uiSize > 0 );

	for ( TUINT i = 0; i < m_uiSize; i++ )
	{
		if ( !m_ppProjectiles[ i ]->IsActive() )
		{
			return m_ppProjectiles[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 007d5d40
void AProjectilePool::KillAll()
{
	TASSERT( m_uiSize > 0 );

	for ( TUINT i = 0; i < m_uiSize; i++ )
	{
		if ( m_ppProjectiles[ i ] )
		{
			delete m_ppProjectiles[ i ];
		}
	}

	::operator delete( m_ppProjectiles );
}

// $deBlob: FUNCTION 007d5e20
void AProjectilePool::KillAllActive( TBOOL a_bPlayEffects )
{
	TASSERT( m_uiSize > 0 );

	for ( TUINT i = 0; i < m_uiSize; i++ )
	{
		if ( m_ppProjectiles[ i ]->IsActive() )
		{
			m_ppProjectiles[ i ]->Kill( a_bPlayEffects );
		}
	}
}

// $deBlob: FUNCTION 007d5db0
void AProjectilePool::Tick( TFLOAT a_fDeltaTime )
{
	TASSERT( m_uiSize > 0 );

	for ( TUINT i = 0; i < m_uiSize; i++ )
	{
		if ( m_ppProjectiles[ i ]->IsActive() )
		{
			m_ppProjectiles[ i ]->Tick( a_fDeltaTime );
		}
	}
}
