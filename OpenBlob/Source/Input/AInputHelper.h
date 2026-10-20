#pragma once
#include "AInputMap.h"
#include "AInputManager2.h"

//-----------------------------------------------------------------------------
// Purpose: the "just pressed" state of game buttons: a state adds the buttons it wants, AddMapping copies their doodads
// from the current context's button map, Update polls them and IsJustDown tells if one went down this frame (or
// repeated). The buttons only react while the context they were added in is the current one.
//-----------------------------------------------------------------------------
class AInputHelper
{
public:
	typedef TUINT8 FLAGS;
	enum FLAGS_ : FLAGS
	{
		FLAGS_JustDown = BITFLAG( 0 ),
		FLAGS_Down     = BITFLAG( 1 ),
		FLAGS_Repeat   = BITFLAG( 2 ),
	};

	struct ButtonInfo
	{
		TINT   m_iDoodad;        // 0x0
		FLAGS  m_uiFlags;        // 0x4
		TFLOAT m_fRepeatTime;    // 0x8
		TFLOAT m_fCurRepeatTime; // 0xC
	};

	// The result of IsJustDownWithResult: the device that pressed the button (an empty handle for the keyboard)
	struct ButtonResult
	{
		TBOOL                              m_bDown;  // 0x0
		AInputManager2::AInputDeviceHandle m_Handle; // 0x4
	};

	struct ButtonMapComparator
	{
		TINT operator()( TUINT32 a, TUINT32 b ) const
		{
			if ( a < b ) return -1;
			return ( a == b ) ? 0 : 1;
		}
	};

	using ButtonInfoVector = Toshi::T2Vector<ButtonInfo, 6>;
	using ButtonMap        = Toshi::T2Map<TUINT32, ButtonInfoVector, ButtonMapComparator>;

public:
	AInputHelper();
	virtual ~AInputHelper();

	// Removes every button
	void Destroy();

	// Adds a_eButton for a_eDevice with the doodads it has in the current context; a_bRepeat makes it "just down"
	// again every a_fRepeatTime seconds while it's held
	void AddMapping( AInputMap::INPUTBUTTON a_eButton, AInputManager2::INPUTDEVICE a_eDevice, TBOOL a_bRepeat, TFLOAT a_fRepeatTime );

	// Adds a_eButton for every controller and the keyboard
	void AddMapping( AInputMap::INPUTBUTTON a_eButton, TBOOL a_bRepeat, TFLOAT a_fRepeatTime );

	// Clears the "just down" state of every button and marks them down, so a button held while it was added doesn't
	// count as pressed
	void ResetJustDown();

	void Update( TFLOAT a_fDeltaTime );

	TBOOL        IsJustDown( AInputMap::INPUTBUTTON a_eButton, AInputManager2::INPUTDEVICE a_eDevice );
	TBOOL        IsJustDown( AInputMap::INPUTBUTTON a_eButton );
	ButtonResult IsJustDownWithResult( AInputMap::INPUTBUTTON a_eButton );

	static TUINT32 MakeButtonDevice( AInputMap::INPUTBUTTON a_eButton, AInputManager2::INPUTDEVICE a_eDevice );
	static void    GetInputButtonDevice( TUINT32 a_uiButtonDevice, AInputMap::INPUTBUTTON& a_rButton, AInputManager2::INPUTDEVICE& a_rDevice );

private:
	void UpdateButtonInfo( Toshi::TInputDevice* a_pDevice, ButtonInfo* a_pButtonInfo );
	void UpdateButtonInfo( ButtonInfo* a_pButtonInfo, AInputManager2::INPUTDEVICE a_eDevice );

private:
	AInputMap::INPUTCONTEXT m_eInputContext; // 0x04 the context of the buttons (set by the first AddMapping)
	ButtonMap               m_oButtonMap;    // 0x08 MakeButtonDevice -> the doodads of the button
};

TSTATICASSERT( sizeof( AInputHelper ) == 0x24 );
