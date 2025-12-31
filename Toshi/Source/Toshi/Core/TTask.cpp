#include "ToshiPCH.h"
#include "TTask.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f2e90
TTask::TTask()
{}

// $deBlob: FUNCTION 006f2ed0
TTask::~TTask()
{
	TASSERT(IsLinked() == TFALSE);
	/*if (m_Scheduler)
		{
			m_Scheduler->DeleteTask();
		}*/
}

// $deBlob: FUNCTION 006f2f30
TBOOL TTask::Create()
{
	TASSERT(IsCreated() == TFALSE);

	if (!IsCreated())
	{
		if (!OnCreate())
		{
			OnDestroy();
			m_Tree->Remove(this, TFALSE);
			Delete();

			return TFALSE;
		}

		auto oldState = m_State;
		m_State |= State_Created;
		Activate(TTRUE);
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f2fc0
TBOOL TTask::CreateFailed()
{
	TASSERT(IsCreated() == TFALSE);

	if (!IsCreated())
	{
		m_Tree->Remove(this, TFALSE);
		Delete();
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006f30b0
TBOOL TTask::Reset()
{
	TTask* firstAttached = Attached();
	TTask* node          = firstAttached;
	TBOOL  result        = TTRUE;

	while (node != TNULL && node != firstAttached)
	{
		result &= node->Reset();
		node = node->Next();
	}

	return result;
}

// $deBlob: FUNCTION 006f30e0
TBOOL TTask::OnCreate()
{
	return TTRUE;
}

// $deBlob: FUNCTION 006f30f0
TBOOL TTask::OnUpdate(TFLOAT deltaTime)
{
	return TTRUE;
}

// $deBlob: FUNCTION 006f3100
void TTask::OnPreDestroy()
{}

// $deBlob: FUNCTION 006f3110
void TTask::OnDestroy()
{}

// $deBlob: FUNCTION 006f3120
TBOOL TTask::OnChildDying(TTask* child)
{
	return TTRUE;
}

// $deBlob: FUNCTION 006f3130
void TTask::OnChildDied(TClass* pClass, TTask* deletedTask)
{}

// $deBlob: FUNCTION 006f3140
void TTask::OnActivate()
{}

// $deBlob: FUNCTION 006f3150
void TTask::OnDeactivate()
{}

void TTask::Activate(TBOOL activate)
{
	TUINT8 oldState = m_State;
	TUINT8 newFlags = activate ? State_Active : 0;

	m_State = (m_State & ~State_Active) | newFlags;

	if (oldState != m_State)
	{
		if (activate)
		{
			OnActivate();
		}
		else
		{
			OnDeactivate();
		}
	}
}

void TTask::AttachTo(TTask* a_pAttachTo)
{
	if (a_pAttachTo != Parent() && a_pAttachTo != TNULL)
	{
		Tree()->ReInsert(a_pAttachTo, this);
	}
}

TOSHI_NAMESPACE_END
