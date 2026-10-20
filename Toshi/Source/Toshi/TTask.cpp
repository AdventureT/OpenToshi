#include "ToshiPCH.h"
#include "TTask.h"
#include "TScheduler.h"
#include "TSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f2e40
// $deBlob: FUNCTION 006f2e50
// $deBlob: FUNCTION 006f2e60
// $deBlob: FUNCTION 006f2e70
// $deBlob: FUNCTION 006f2e80
// $deBlob: FUNCTION 0053f910
// $deBlob: FUNCTION 008594a0
TDEFINE_CLASS_NORUNTIME( TTask );

// $deBlob: FUNCTION 006f2e90
// $Barnyard: FUNCTION 006bc910
TTask::TTask()
{
	m_ulFlags = 0;
}

// $deBlob: FUNCTION 006f2ed0
// $Barnyard: FUNCTION 006bc8c0
TTask::~TTask()
{
	TASSERT( IsLinked() == TFALSE );
}

// $deBlob: FUNCTION 006f2f20
TScheduler* TTask::GetScheduler()
{
	return TSystemManager::GetSingleton()->GetScheduler();
}

// $deBlob: FUNCTION 006f2f30
// $Barnyard: FUNCTION 006bc940
TBOOL TTask::Create()
{
	TASSERT( IsCreated() == TFALSE );

	if ( !IsCreated() )
	{
		if ( !OnCreate() )
		{
			OnDestroy();
			m_Tree->Remove( this, TFALSE );

			// NOTE: de Blob calls the deleting destructor (Barnyard: Delete())
			delete this;

			return TFALSE;
		}

		m_ulFlags |= FLAGS_CREATED;
		Activate( TTRUE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f2fc0
// $Barnyard: FUNCTION 006bc990
TBOOL TTask::CreateFailed()
{
	TASSERT( IsCreated() == TFALSE );

	if ( !IsCreated() )
	{
		m_Tree->Remove( this, TFALSE );
		delete this;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006f30b0
// $Barnyard: FUNCTION 006bc8e0
TBOOL TTask::Reset()
{
	TTask* firstAttached = Child();
	TTask* node          = firstAttached;
	TBOOL  result        = TTRUE;

	// NOTE: the Barnyard port checked node != firstAttached before the first iteration, so it never reset anything
	if ( node != TNULL )
	{
		do
		{
			result &= node->Reset();
			node = node->Next();
		} while ( node != firstAttached && node != TNULL );
	}

	return result;
}

// $deBlob: FUNCTION 006f30e0
TBOOL TTask::OnCreate()
{
	return TTRUE;
}

// $deBlob: FUNCTION 006f30f0
TBOOL TTask::OnUpdate( TFLOAT a_fDeltaTime )
{
	return TTRUE;
}

// $deBlob: FUNCTION 006f3100
void TTask::OnPreDestroy()
{
}

// $deBlob: FUNCTION 006f3110
void TTask::OnDestroy()
{
}

// $deBlob: FUNCTION 006f3120
TBOOL TTask::OnChildDying( TTask* child )
{
	return TTRUE;
}

// $deBlob: FUNCTION 006f3130
void TTask::OnChildDied( const TClass* pClass, TTask* deletedTask )
{
}

// $deBlob: FUNCTION 006f3140
void TTask::OnActivate()
{
}

// $deBlob: FUNCTION 006f3150
void TTask::OnDeactivate()
{
}

// $deBlob: FUNCTION 006f3060
// $Barnyard: FUNCTION 006bc850
void TTask::Activate( TBOOL activate )
{
	TUINT32 oldState = m_ulFlags;
	TUINT32 newFlags = activate == TTRUE ? FLAGS_ACTIVE : 0;

	m_ulFlags = ( m_ulFlags & ~FLAGS_ACTIVE ) | newFlags;

	if ( oldState != m_ulFlags )
	{
		if ( activate == TTRUE )
		{
			OnActivate();
		}
		else
		{
			OnDeactivate();
		}
	}
}

// $deBlob: FUNCTION 006f30a0
void TTask::Destroy()
{
	TSystemManager::GetSingleton()->GetScheduler()->KillTask( this );
}

void TTask::SetParent( TTask* a_pAttachTo )
{
	if ( this != Parent() && a_pAttachTo != TNULL )
	{
		Tree()->ReInsert( this != Parent() ? a_pAttachTo : TNULL, this );
	}
}

TOSHI_NAMESPACE_END
