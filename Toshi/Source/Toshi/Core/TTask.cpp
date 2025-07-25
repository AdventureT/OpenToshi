#include "ToshiPCH.h"
#include "TTask.h"

TOSHI_NAMESPACE_START

TTask::TTask()
{}

TTask::~TTask()
{
	TASSERT(IsLinked() == TFALSE);
	/*if (m_Scheduler)
		{
			m_Scheduler->DeleteTask();
		}*/
}

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

TBOOL TTask::OnCreate()
{
	return TTRUE;
}

TBOOL TTask::OnUpdate(TFLOAT deltaTime)
{
	return TTRUE;
}

void TTask::OnPreDestroy()
{}

void TTask::OnDestroy()
{}

TBOOL TTask::OnChildDying(TTask* child)
{
	return TTRUE;
}

void TTask::OnChildDied(TClass* pClass, TTask* deletedTask)
{}

void TTask::OnActivate()
{}

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
