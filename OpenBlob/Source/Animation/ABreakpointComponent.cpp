#include "pch.h"
#include "ABreakpointComponent.h"
#include "AModelInstance2.h"
#include "AToshiAnimInterface2.h"

#include <Render/TSkeleton.h>
#include <Render/TAnimation.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0056ea60
ABreakpointComponent::ABreakpointComponent( AModelInstance2* a_pModelInstance )
    : m_pModelInstance( a_pModelInstance )
{
}

// $deBlob: FUNCTION 0056ea90
// $deBlob: FUNCTION 0056f1b0
ABreakpointComponent::~ABreakpointComponent()
{
	for ( auto it = m_Breakpoints.Begin(); it != m_Breakpoints.End(); it++ )
	{
		while ( !it->m_Events.IsEmpty() )
			delete it->m_Events.PopFront();
	}

	while ( !m_Breakpoints.IsEmpty() )
	{
		Breakpoint* pBreakpoint = m_Breakpoints.PopFront();

		if ( pBreakpoint != TNULL )
			delete pBreakpoint;
	}
}

// $deBlob: FUNCTION 0056ebd0
void ABreakpointComponent::AddBreakpoint( TINT a_iSequence, TFLOAT a_fTime, t_Callback a_fnCallback, void* a_pUserData, TBOOL a_bAlways )
{
	Breakpoint* pBreakpoint = TNULL;

	for ( auto it = m_Breakpoints.Begin(); it != m_Breakpoints.End(); it++ )
	{
		if ( it->m_iSequence == a_iSequence )
		{
			pBreakpoint = it;
			break;
		}
	}

	if ( pBreakpoint == TNULL )
	{
		pBreakpoint              = new Breakpoint;
		pBreakpoint->m_iSequence = TINT16( a_iSequence );
		m_Breakpoints.PushBack( pBreakpoint );
	}

	Event* pEvent        = new Event;
	pEvent->m_fnCallback = a_fnCallback;
	pEvent->m_bAlways    = a_bAlways;
	pEvent->m_fTime      = a_fTime;
	pEvent->m_pUserData  = a_pUserData;
	pBreakpoint->m_Events.PushBack( pEvent );
}

// $deBlob: FUNCTION 0056ed00
void ABreakpointComponent::TickImp()
{
	TASSERT( !m_Breakpoints.IsEmpty() );

	AToshiAnimInterface2* pAnimInterface = m_pModelInstance->GetAnimInterface();

	for ( auto it = m_Breakpoints.Begin(); it != m_Breakpoints.End(); it++ )
	{
		if ( it->m_iSequence < 0 || pAnimInterface->GetSkeletonInstance() == TNULL )
			continue;

		TAnimation* pAnimation = pAnimInterface->GetSkeletonInstance()->IsAnimating( it->m_iSequence );

		if ( pAnimation == TNULL )
			continue;

		TFLOAT fDeltaTime = TSystemManager::GetSingleton()->GetScheduler()->GetCurrentDeltaTime();

		for ( auto itEvent = it->m_Events.Begin(); itEvent != it->m_Events.End(); itEvent++ )
		{
			TFLOAT fDuration = pAnimation->GetSkeletonInstance()->GetSkeleton()->GetSequence( pAnimation->GetSequence() )->GetDuration();

			if ( itEvent->m_bAlways || pAnimation->GetTotalTime() <= fDuration )
			{
				TFLOAT fTime = itEvent->m_fTime;

				if ( pAnimation->GetSeqTime() <= fTime && fTime <= pAnimation->GetSpeed() * fDeltaTime + pAnimation->GetSeqTime() )
					itEvent->m_fnCallback( it->m_iSequence, fTime, itEvent->m_pUserData );
			}
		}
	}
}
