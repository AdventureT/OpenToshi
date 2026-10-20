#pragma once
#include <Toshi/T2DList.h>

//-----------------------------------------------------------------------------
// Purpose: a job of an AJobQueue (..\..\Source\JobSystem): the queue begins it, runs it once per frame until it's
// finished (SetReady), its owner reads the result and releases it (SetDone).
//-----------------------------------------------------------------------------
class AJob
    : public Toshi::T2DList<AJob>::Node
{
public:
	// m_uiFlags
	enum FLAG : TUINT8
	{
		FLAG_Ready = 0x1, // Run finished
		FLAG_Done  = 0x2, // the owner has read the result: back to the free list
	};

public:
	AJob();
	virtual ~AJob(); // +0x00

	virtual void  SetReady();  // +0x04
	virtual void  SetDone();   // +0x08
	virtual TBOOL Run()   = 0; // +0x0C TTRUE: finished
	virtual void  Begin() = 0; // +0x10

	TBOOL IsReady() const { return m_uiFlags & FLAG_Ready; }
	TBOOL IsDone() const { return m_uiFlags & FLAG_Done; }
	void  ClearFlags() { m_uiFlags = 0; }

protected:
	TUINT8 m_uiFlags; // 0x0C FLAG
};
