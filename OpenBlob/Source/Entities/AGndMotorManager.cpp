#include "pch.h"
#include "AGndMotorManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00790100
AGndMotorManager::AGndMotorManager()
    : m_itNext( TSTATICCAST( AGndMotorNode, TNULL ) )
{
}

// $deBlob: FUNCTION 00790190
// $deBlob: FUNCTION 005c86e0
AGndMotorManager::~AGndMotorManager()
{
	m_llMotors.Clear();
}

// $deBlob: FUNCTION 00790220
void AGndMotorManager::RegisterMotorComponent( AGndMotorComponent* a_pMotor )
{
	TBOOL bWasEmpty = m_llMotors.IsEmpty();
	m_llMotors.PushBack( &a_pMotor->m_oMotorNode );

	if ( bWasEmpty )
		m_itNext = m_llMotors.Begin();
}

// $deBlob: FUNCTION 00790270
void AGndMotorManager::UnregisterMotorComponent( AGndMotorComponent* a_pMotor )
{
	for ( auto it = m_llMotors.Begin(); it != m_llMotors.End(); ++it )
	{
		if ( it->GetMotor() == a_pMotor )
		{
			if ( it == m_itNext )
			{
				// Moves the round-robin past the removed motor (and past the list's root)
				++m_itNext;

				if ( m_itNext == m_llMotors.End() )
					++m_itNext;
			}

			m_llMotors.Erase( it );
			return;
		}
	}
}

// $deBlob: FUNCTION 00790320
void AGndMotorManager::Update( TFLOAT a_fDeltaTime )
{
	if ( m_llMotors.IsEmpty() )
		return;

	auto it = m_itNext;
	++it;

	while ( it != m_itNext )
	{
		if ( it != m_llMotors.End() && HASANYFLAG( it->GetMotor()->m_uiFlags, AGndMotorComponent::FLAG_AtRest ) )
		{
			// The resting motor probes its ground contact again on its next tick
			it->GetMotor()->m_uiFlags |= AGndMotorComponent::FLAG_ProbeGround;
			m_itNext = it;
			return;
		}

		++it;
	}

	m_itNext = it;
}

// $deBlob: FUNCTION 00790430
void AGndMotorManager::OnCollisionObjectDestroyed( ACollisionObject* a_pObject )
{
	for ( auto it = m_llMotors.Begin(); it != m_llMotors.End(); ++it )
		it->GetMotor()->OnCollisionObjectDestroyed( a_pObject );
}
