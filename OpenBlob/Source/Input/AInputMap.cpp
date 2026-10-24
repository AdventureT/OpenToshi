#include "pch.h"
#include "AInputMap.h"

#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Binds the doodads to a button of a button map (retail pushes them into a local vector, inserts it and clears it)
static void AInputMap_Bind( AInputMap::ButtonMap& a_rMap, AInputMap::INPUTBUTTON a_eButton, std::initializer_list<TINT> a_Doodads )
{
	AInputMap::BindVector oBinds;

	for ( TINT iDoodad : a_Doodads )
		oBinds.PushBack( iDoodad );

	a_rMap.Insert( a_eButton, oBinds );
}

// $deBlob: FUNCTION 005dccb0
AInputMap::AInputMap()
{
	InitialiseButtonMap();
	CheckButtonMaps();
	NamePadDoodads();
}

// $deBlob: FUNCTION 005dcde0
// $deBlob: FUNCTION 005dd500
AInputMap::~AInputMap()
{
	for ( TINT i = 0; i < INPUTCONTEXT_NUMOF; i++ )
		m_aButtonMaps[ i ].Clear();

	m_oNamePadDoodads.Clear();
}

// $deBlob: FUNCTION 005dd0b0
AInputMap::ButtonMap* AInputMap::GetButtonMap( INPUTCONTEXT a_eContext )
{
	if ( a_eContext >= 0 && a_eContext < INPUTCONTEXT_NUMOF )
		return &m_aButtonMaps[ a_eContext ];

	return TNULL;
}

// $deBlob: FUNCTION 005dd150
TINT AInputMap::GetButtonLocaleID( ButtonMap* a_pButtonMap, INPUTBUTTON a_eButton )
{
	auto it = a_pButtonMap->Find( a_eButton );

	if ( it == a_pButtonMap->End() )
		return -1;

	BindVector oBinds = it->GetSecond();

	// NOTE: retail doesn't check that the doodad has a name (an unnamed one returns the map's element count)
	return m_oNamePadDoodads.Find( *oBinds.Begin() )->GetSecond();
}

// Checks the button maps (the checks are compiled out in retail: only the loops remain)
// $deBlob: FUNCTION 005dd200
void AInputMap::CheckButtonMaps()
{
	for ( TINT i = 0; i < INPUTCONTEXT_NUMOF; i++ )
	{
		ButtonMap* pMap = GetButtonMap( i );

		for ( auto it = pMap->Begin(); it != pMap->End(); it++ )
		{
			auto it2 = it;

			for ( it2++; it2 != pMap->End(); it2++ )
			{
			}
		}
	}
}

// $deBlob: FUNCTION 005dd960
void AInputMap::InitialiseButtonMap()
{
	// Context 0
	AInputMap_Bind( m_aButtonMaps[ 0 ], 0, { TInputDeviceKeyboard::KEY_ESCAPE, TInputDeviceController::BUTTON_CONTEXT_1 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 1, { TInputDeviceKeyboard::KEY_SPACE, TInputDeviceController::BUTTON_CONTEXT_3 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 2, { TInputDeviceKeyboard::KEY_UP, TInputDeviceController::BUTTON_DPAD_UP, TInputDeviceController::VIRTBTN_STICK_LEFT_UP } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 3, { TInputDeviceKeyboard::KEY_DOWN, TInputDeviceController::BUTTON_DPAD_DOWN, TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 4, { TInputDeviceKeyboard::KEY_LEFT, TInputDeviceController::BUTTON_DPAD_LEFT, TInputDeviceController::VIRTBTN_STICK_LEFT_LEFT } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 5, { TInputDeviceKeyboard::KEY_RIGHT, TInputDeviceController::BUTTON_DPAD_RIGHT, TInputDeviceController::VIRTBTN_STICK_LEFT_RIGHT } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 6, { TInputDeviceKeyboard::KEY_RETURN, TInputDeviceKeyboard::KEY_RBRACKET, TInputDeviceController::BUTTON_START } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 8, { TInputDeviceKeyboard::KEY_PERIOD, TInputDeviceController::BUTTON_SHOULDER_RIGHT_2 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 9, { TInputDeviceKeyboard::KEY_COMMA, TInputDeviceController::BUTTON_SHOULDER_LEFT_2 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 10, { TInputDeviceKeyboard::KEY_F2 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 20, { TInputDeviceKeyboard::KEY_GRAVE } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 21, { TInputDeviceKeyboard::KEY_T, TInputDeviceController::BUTTON_CONTEXT_1 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 22, { TInputDeviceController::BUTTON_DPAD_LEFT, TInputDeviceKeyboard::KEY_Y } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 23, { TInputDeviceKeyboard::KEY_R } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 26, { TInputDeviceKeyboard::KEY_F1 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 27, { TInputDeviceKeyboard::KEY_F5 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 18, { TInputDeviceKeyboard::KEY_F6 } );
	AInputMap_Bind( m_aButtonMaps[ 0 ], 19, { TInputDeviceKeyboard::KEY_F7 } );

	// Context 1
	AInputMap_Bind( m_aButtonMaps[ 1 ], 20, { TInputDeviceKeyboard::KEY_GRAVE, TInputDeviceKeyboard::KEY_ESCAPE, TInputDeviceController::BUTTON_CONTEXT_1 } );
	AInputMap_Bind( m_aButtonMaps[ 1 ], 24, { TInputDeviceKeyboard::KEY_BACK } );
	AInputMap_Bind( m_aButtonMaps[ 1 ], 2, { TInputDeviceKeyboard::KEY_UP, TInputDeviceController::BUTTON_DPAD_UP, TInputDeviceController::VIRTBTN_STICK_LEFT_UP } );
	AInputMap_Bind( m_aButtonMaps[ 1 ], 3, { TInputDeviceKeyboard::KEY_DOWN, TInputDeviceController::BUTTON_DPAD_DOWN, TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN } );
	AInputMap_Bind( m_aButtonMaps[ 1 ], 25, { TInputDeviceKeyboard::KEY_TAB } );
	AInputMap_Bind( m_aButtonMaps[ 1 ], 28, { TInputDeviceKeyboard::KEY_RETURN } );
	AInputMap_Bind( m_aButtonMaps[ 1 ], 29, { TInputDeviceKeyboard::KEY_A } );

	// Context 2
	AInputMap_Bind( m_aButtonMaps[ 2 ], 6, { TInputDeviceKeyboard::KEY_RBRACKET, TInputDeviceController::BUTTON_START } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 16, { TInputDeviceController::BUTTON_START } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 7, { TInputDeviceController::BUTTON_SELECT, TInputDeviceKeyboard::KEY_LBRACKET } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 12, { TInputDeviceController::BUTTON_CONTEXT_3, TInputDeviceKeyboard::KEY_SPACE } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 13, { TInputDeviceKeyboard::KEY_RETURN, TInputDeviceController::BUTTON_CONTEXT_4 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 14, { TInputDeviceKeyboard::KEY_RSHIFT, TInputDeviceController::BUTTON_CONTEXT_2 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 15, { TInputDeviceKeyboard::KEY_Y, TInputDeviceController::BUTTON_CONTEXT_1 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 17, { TInputDeviceKeyboard::KEY_LMENU, TInputDeviceController::BUTTON_SHOULDER_RIGHT_2 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 0, { TInputDeviceKeyboard::KEY_ESCAPE } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 10, { TInputDeviceKeyboard::KEY_F2 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 11, { TInputDeviceKeyboard::KEY_F3, TInputDeviceController::BUTTON_SHOULDER_LEFT_2 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 20, { TInputDeviceKeyboard::KEY_GRAVE } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 18, { TInputDeviceKeyboard::KEY_F6 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 19, { TInputDeviceKeyboard::KEY_F7 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 31, { TInputDeviceKeyboard::KEY_F } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 32, { TInputDeviceKeyboard::KEY_C } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 33, { TInputDeviceKeyboard::KEY_LCONTROL } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 34, { TInputDeviceKeyboard::KEY_LMENU } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 30, { TInputDeviceKeyboard::KEY_F4 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 36, { TInputDeviceKeyboard::KEY_F8 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 37, { TInputDeviceKeyboard::KEY_F9 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 38, { TInputDeviceKeyboard::KEY_F10 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 39, { TInputDeviceKeyboard::KEY_F11 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 21, { TInputDeviceKeyboard::KEY_T, TInputDeviceController::BUTTON_CONTEXT_1 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 22, { TInputDeviceController::BUTTON_DPAD_LEFT, TInputDeviceKeyboard::KEY_Y } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 23, { TInputDeviceKeyboard::KEY_R, TInputDeviceController::BUTTON_CONTEXT_2 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 26, { TInputDeviceKeyboard::KEY_F1 } );
	AInputMap_Bind( m_aButtonMaps[ 2 ], 27, { TInputDeviceKeyboard::KEY_F5 } );

	// Context 3
	AInputMap_Bind( m_aButtonMaps[ 3 ], 35, { TInputDeviceKeyboard::KEY_SPACE, TInputDeviceKeyboard::KEY_ESCAPE, TInputDeviceController::BUTTON_CONTEXT_3, TInputDeviceController::BUTTON_CONTEXT_1, TInputDeviceController::BUTTON_START } );

	// Context 4
	AInputMap_Bind( m_aButtonMaps[ 4 ], 1, { TInputDeviceController::BUTTON_START, TInputDeviceKeyboard::KEY_RETURN } );

	// Context 5
	AInputMap_Bind( m_aButtonMaps[ 5 ], 40, { TInputDeviceController::BUTTON_CONTEXT_3, TInputDeviceKeyboard::KEY_RIGHT } );
	AInputMap_Bind( m_aButtonMaps[ 5 ], 41, { TInputDeviceController::BUTTON_CONTEXT_2, TInputDeviceKeyboard::KEY_LEFT } );
	AInputMap_Bind( m_aButtonMaps[ 5 ], 0, { TInputDeviceController::BUTTON_CONTEXT_1, TInputDeviceKeyboard::KEY_ESCAPE } );

	// Context 6
	AInputMap_Bind( m_aButtonMaps[ 6 ], 0, { TInputDeviceController::BUTTON_CONTEXT_1, TInputDeviceKeyboard::KEY_RETURN, TInputDeviceController::BUTTON_CONTEXT_3, TInputDeviceKeyboard::KEY_ESCAPE } );
	AInputMap_Bind( m_aButtonMaps[ 6 ], 2, { TInputDeviceController::BUTTON_DPAD_UP, TInputDeviceController::VIRTBTN_STICK_LEFT_UP, TInputDeviceKeyboard::KEY_UP } );
	AInputMap_Bind( m_aButtonMaps[ 6 ], 3, { TInputDeviceController::BUTTON_DPAD_DOWN, TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN, TInputDeviceKeyboard::KEY_DOWN } );

	// Context 7
	AInputMap_Bind( m_aButtonMaps[ 7 ], 42, { TInputDeviceController::BUTTON_CONTEXT_3, TInputDeviceController::BUTTON_CONTEXT_1, TInputDeviceKeyboard::KEY_SPACE } );
	AInputMap_Bind( m_aButtonMaps[ 7 ], 43, { TInputDeviceController::BUTTON_START, TInputDeviceKeyboard::KEY_RETURN } );

	// Context 8
	AInputMap_Bind( m_aButtonMaps[ 8 ], 44, { TInputDeviceKeyboard::KEY_A, TInputDeviceController::BUTTON_SHOULDER_LEFT_1 } );
	AInputMap_Bind( m_aButtonMaps[ 8 ], 45, { TInputDeviceKeyboard::KEY_D, TInputDeviceController::BUTTON_SHOULDER_RIGHT_1 } );
	// NOTE: retail doesn't clear the vector after the previous binding, so D and the right shoulder button also
	// cancel in this context
	AInputMap_Bind( m_aButtonMaps[ 8 ], 0, { TInputDeviceKeyboard::KEY_D, TInputDeviceController::BUTTON_SHOULDER_RIGHT_1, TInputDeviceKeyboard::KEY_ESCAPE, TInputDeviceController::BUTTON_CONTEXT_1 } );
	AInputMap_Bind( m_aButtonMaps[ 8 ], 1, { TInputDeviceKeyboard::KEY_SPACE, TInputDeviceController::BUTTON_CONTEXT_3 } );

	// Context 0
	AInputMap_Bind( m_aButtonMaps[ 0 ], 46, { TInputDeviceController::VIRTSTK_DPAD } );

	// Context 9
	AInputMap_Bind( m_aButtonMaps[ 9 ], 2, { TInputDeviceController::BUTTON_DPAD_UP, TInputDeviceController::VIRTBTN_STICK_LEFT_UP, TInputDeviceKeyboard::KEY_UP } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 3, { TInputDeviceController::BUTTON_DPAD_DOWN, TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN, TInputDeviceKeyboard::KEY_DOWN } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 4, { TInputDeviceController::BUTTON_DPAD_LEFT, TInputDeviceController::VIRTBTN_STICK_LEFT_LEFT, TInputDeviceKeyboard::KEY_LEFT } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 5, { TInputDeviceController::BUTTON_DPAD_RIGHT, TInputDeviceController::VIRTBTN_STICK_LEFT_RIGHT, TInputDeviceKeyboard::KEY_RIGHT } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 48, { TInputDeviceController::BUTTON_CONTEXT_4, TInputDeviceKeyboard::KEY_E } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 0, { TInputDeviceController::BUTTON_CONTEXT_1, TInputDeviceKeyboard::KEY_ESCAPE } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 1, { TInputDeviceController::BUTTON_CONTEXT_2, TInputDeviceKeyboard::KEY_SPACE } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 49, { TInputDeviceController::BUTTON_CONTEXT_3, TInputDeviceKeyboard::KEY_A } );
	AInputMap_Bind( m_aButtonMaps[ 9 ], 47, { TInputDeviceController::STICK_LEFT } );
}

// $deBlob: FUNCTION 005dedf0
void AInputMap::NamePadDoodads()
{
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_DPAD_UP, 0x75E );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_DPAD_RIGHT, 0x75F );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_DPAD_DOWN, 0x760 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_DPAD_LEFT, 0x761 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_SELECT, 0x762 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_START, 0x763 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_STICK_LEFT, 0x764 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_STICK_RIGHT, 0x765 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_CONTEXT_1, 0x766 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_CONTEXT_2, 0x767 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_CONTEXT_3, 0x768 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_CONTEXT_4, 0x769 );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_SHOULDER_LEFT_1, 0x76A );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_SHOULDER_LEFT_2, 0x76B );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_SHOULDER_RIGHT_1, 0x76C );
	m_oNamePadDoodads.Insert( TInputDeviceController::BUTTON_SHOULDER_RIGHT_2, 0x76D );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_LEFT_UP, 0x76E );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_LEFT_RIGHT, 0x76F );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_LEFT_DOWN, 0x770 );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_LEFT_LEFT, 0x771 );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_RIGHT_UP, 0x772 );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_RIGHT_RIGHT, 0x773 );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_RIGHT_DOWN, 0x774 );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTBTN_STICK_RIGHT_LEFT, 0x775 );
	m_oNamePadDoodads.Insert( TInputDeviceController::VIRTSTK_DPAD, 0x778 );
	m_oNamePadDoodads.Insert( TInputDeviceController::STICK_LEFT, 0x776 );
	m_oNamePadDoodads.Insert( TInputDeviceController::STICK_RIGHT, 0x777 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_LSHIFT, 0x7A2 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_RSHIFT, 0x7AE );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_LCONTROL, 0x795 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_RCONTROL, 0x7DF );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_LMENU, 0x7B0 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_RMENU, 0x7E3 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_ESCAPE, 0x779 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_1, 0x77A );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_2, 0x77B );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_3, 0x77C );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_4, 0x77D );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_5, 0x77E );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_6, 0x77F );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_7, 0x780 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_8, 0x781 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_9, 0x782 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_0, 0x783 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_MINUS, 0x784 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_EQUALS, 0x785 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_BACK, 0x786 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_TAB, 0x787 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_Q, 0x788 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_W, 0x789 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_E, 0x78A );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_R, 0x78B );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_T, 0x78C );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_Y, 0x78D );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_U, 0x78E );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_I, 0x78F );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_O, 0x790 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_P, 0x791 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_LBRACKET, 0x792 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_RBRACKET, 0x793 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_RETURN, 0x794 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_A, 0x796 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_S, 0x797 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_D, 0x798 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F, 0x799 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_G, 0x79A );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_H, 0x79B );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_J, 0x79C );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_K, 0x79D );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_L, 0x79E );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_SEMICOLON, 0x79F );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_APOSTROPHE, 0x7A0 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_GRAVE, 0x7A1 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_BACKSLASH, 0x7A3 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_Z, 0x7A4 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_X, 0x7A5 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_C, 0x7A6 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_V, 0x7A7 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_B, 0x7A8 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_N, 0x7A9 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_M, 0x7AA );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_COMMA, 0x7AB );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_PERIOD, 0x7AC );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_SLASH, 0x7AD );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_MULTIPLY, 0x7AF );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_SPACE, 0x7B1 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_CAPITAL, 0x7B2 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F1, 0x7B3 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F2, 0x7B4 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F3, 0x7B5 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F4, 0x7B6 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F5, 0x7B7 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F6, 0x7B8 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F7, 0x7B9 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F8, 0x7BA );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F9, 0x7BB );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F10, 0x7BC );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMLOCK, 0x7BD );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_SCROLL, 0x7BE );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD7, 0x7BF );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD8, 0x7C0 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD9, 0x7C1 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_SUBTRACT, 0x7C2 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD4, 0x7C3 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD5, 0x7C4 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD6, 0x7C5 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_ADD, 0x7C6 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD1, 0x7C7 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD2, 0x7C8 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD3, 0x7C9 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPAD0, 0x7CA );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_DECIMAL, 0x7CB );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F11, 0x7CC );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F12, 0x7CD );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F13, 0x7CE );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F14, 0x7CF );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_F15, 0x7D0 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_KANA, 0x7D1 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_CONVERT, 0x7D2 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NOCONVERT, 0x7D3 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_YEN, 0x7D4 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPADEQUALS, 0x7D5 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_CIRCUMFLEX, 0x7D6 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_AT, 0x7D7 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_COLON, 0x7D8 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_UNDERLINE, 0x7D9 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_KANJI, 0x7DA );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_STOP, 0x7DB );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_AX, 0x7DC );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_UNLABELED, 0x7DD );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPADENTER, 0x7DE );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_NUMPADCOMMA, 0x7E0 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_DIVIDE, 0x7E1 );
	m_oNamePadDoodads.Insert( TInputDeviceKeyboard::KEY_SYSRQ, 0x7E2 );
}
