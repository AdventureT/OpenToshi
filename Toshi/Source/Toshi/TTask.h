#pragma once
#include "TObject.h"
#include "TRefCounted.h"
#include "TNodeTree.h"

TOSHI_NAMESPACE_START

class TScheduler;

class TTask : public TObject
    , public TRefCounted
    , public TNodeTree<TTask>::TNode
{
public:
	TDECLARE_CLASS( TTask, TObject );

public:
	friend TScheduler;

	enum FLAGS
	{
		FLAGS_CREATED = BITFLAG( 0 ),
		FLAGS_ACTIVE  = BITFLAG( 1 ),
		FLAGS_DYING   = BITFLAG( 2 ),
	};

public:
	TTask();
	// NOTE: pure virtual in de Blob (__purecall in TTask's vtable), defined in TTask.cpp
	virtual ~TTask() = 0;

	// The system manager's scheduler
	TScheduler* GetScheduler();

	virtual TBOOL Create();
	virtual TBOOL CreateFailed();
	virtual TBOOL Reset();
	virtual TBOOL OnCreate();
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime );
	virtual void  OnPreDestroy();
	virtual void  OnDestroy();
	virtual TBOOL OnChildDying( TTask* child );
	virtual void  OnChildDied( const TClass* a_pClass, TTask* a_pDeletedTask );
	virtual void  OnActivate();
	virtual void  OnDeactivate();

	void Activate( TBOOL activate );

	// Marks this task and its children as dying
	void Destroy();
	void SetParent( TTask* a_pAttachTo );

	// $deBlob: FUNCTION 006f3050
	TUINT32 GetFlags() const { return m_ulFlags; }
	// $deBlob: FUNCTION 006f3020
	TBOOL IsCreated() const { return m_ulFlags & FLAGS_CREATED; }
	// $deBlob: FUNCTION 006f3030
	// $Barnyard: FUNCTION 006bc8a0
	TBOOL IsActive() const { return m_ulFlags & FLAGS_ACTIVE; }
	TBOOL IsCreatedAndActive() const { return ( m_ulFlags & ( FLAGS_CREATED | FLAGS_ACTIVE ) ) == ( FLAGS_CREATED | FLAGS_ACTIVE ); }
	// $deBlob: FUNCTION 006f3040
	// $Barnyard: FUNCTION 006bc8b0
	TBOOL IsDying() const { return m_ulFlags & FLAGS_DYING; }

private:
	TUINT32 m_ulFlags; // 0x1C
};

TOSHI_NAMESPACE_END
