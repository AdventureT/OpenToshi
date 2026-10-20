#pragma once
#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// Purpose: the pressed/held/released state of a few doodads of one device, without input contexts (AGameState's console
// key)
//-----------------------------------------------------------------------------
class AInputHelperSimple
{
public:
	typedef TINT STATE;
	enum STATE_ : STATE
	{
		STATE_None,
		STATE_Pressed,
		STATE_Held,
		STATE_Released,
	};

	struct DoodadInfo
	{
		TINT  m_iDoodad; // 0x0
		STATE m_eState;  // 0x4
	};

public:
	AInputHelperSimple();
	AInputHelperSimple( TINT a_iArraySize, Toshi::TInputDevice* a_pInputDevice );
	~AInputHelperSimple();

	void Initialise( TINT a_iDoodadCount, Toshi::TInputDevice* a_pInputDevice );
	void AddDoodad( TINT a_iDoodad );

	// The device the doodads are read from
	void                 SetInputDevice( Toshi::TInputDevice* a_pInputDevice ) { m_pInputDevice = a_pInputDevice; }
	Toshi::TInputDevice* GetInputDevice() const { return m_pInputDevice; }
	// $deBlob: FUNCTION 005daa10
	DoodadInfo* GetArray() const { return m_pArray; }
	TINT        GetNumDoodads() const { return m_iArrayIndex; }
	void        Update();
	TBOOL       WasPressed( TINT a_iDoodad );
	TBOOL       IsHeld( TINT a_iDoodad );
	TBOOL       WasReleased( TINT a_iDoodad );

private:
	DoodadInfo*          m_pArray;       // 0x0
	TINT                 m_iArrayIndex;  // 0x4 the number of doodads
	TINT                 m_iArraySize;   // 0x8
	Toshi::TInputDevice* m_pInputDevice; // 0xC
};

TSTATICASSERT( sizeof( AInputHelperSimple ) == 0x10 );
