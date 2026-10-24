#pragma once
#include <Toshi/T2SList.h>

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: calls functions at set times of a model instance's animations. Nothing creates one in retail.
//-----------------------------------------------------------------------------
class ABreakpointComponent
{
public:
	// a_iSequence, the breakpoint's time, the user data
	using t_Callback = void ( * )( TINT a_iSequence, TFLOAT a_fTime, void* a_pUserData );

	// A time of a sequence (0x14 bytes)
	struct Event : public Toshi::T2SList<Event>::Node
	{
		TFLOAT     m_fTime;      // 0x04
		TBOOL      m_bAlways;    // 0x08 also past the sequence's end (looping)
		t_Callback m_fnCallback; // 0x0C
		void*      m_pUserData;  // 0x10
	};

	// The events of a sequence (0x0C bytes)
	struct Breakpoint : public Toshi::T2SList<Breakpoint>::Node
	{
		Toshi::T2SList<Event> m_Events;    // 0x04
		TINT16                m_iSequence; // 0x08
	};

public:
	ABreakpointComponent( AModelInstance2* a_pModelInstance );
	virtual ~ABreakpointComponent(); // +0x00

	// Calls the events whose time the animation of their sequence passes this frame
	void TickImp();

	// Adds an event at a_fTime of the sequence a_iSequence
	void AddBreakpoint( TINT a_iSequence, TFLOAT a_fTime, t_Callback a_fnCallback, void* a_pUserData, TBOOL a_bAlways );

private:
	AModelInstance2*           m_pModelInstance; // 0x04
	Toshi::T2SList<Breakpoint> m_Breakpoints;    // 0x08
};                                               // 0x0C
