#pragma once
#include <Toshi/T2Map.h>
#include <Toshi/T2Vector.h>

//-----------------------------------------------------------------------------
// Purpose: the game's input bindings: for each input context, the doodads (keys and pad buttons) of each game button,
// and the locale string of each doodad (the name shown in the controls menus).
// the contexts and buttons are unnamed there, de Blob's PC build binds up to 6 doodads per button (T2Vector<int, 6>)
// where the Wii one binds 4.
//-----------------------------------------------------------------------------
class AInputMap
{
public:
	// The input contexts (AInputManager2's context stack)
	typedef TINT INPUTCONTEXT;
	enum INPUTCONTEXT_ : INPUTCONTEXT
	{
		INPUTCONTEXT_Menu          = 0, // AXUIState and the pause states
		INPUTCONTEXT_1             = 1,
		INPUTCONTEXT_InGame        = 2, // AInGameStateHelper
		INPUTCONTEXT_Movie         = 3, // AMovieState
		INPUTCONTEXT_4             = 4,
		INPUTCONTEXT_5             = 5,
		INPUTCONTEXT_6             = 6,
		INPUTCONTEXT_FrontEndMovie = 7, // AFrontEndMovieState
		INPUTCONTEXT_8             = 8,
		INPUTCONTEXT_9             = 9,
		INPUTCONTEXT_NUMOF         = 10,
		INPUTCONTEXT_None          = 11, // AInputManager2::GetContext with an empty stack, AInputHelper's "not set yet"
	};

	// The game buttons (the keys of the button maps)
	typedef TINT INPUTBUTTON;

	// The doodads of a button: TInputDeviceKeyboard::KEY_* and TInputDeviceController::BUTTON_* codes
	using BindVector = Toshi::T2Vector<TINT, 6>;

	using ButtonMapComparator = Toshi::TComparator<INPUTBUTTON>;
	using ButtonMap           = Toshi::T2Map<INPUTBUTTON, BindVector, ButtonMapComparator>;
	using NamePadMap          = Toshi::T2Map<TINT, TINT>;

public:
	AInputMap();
	virtual ~AInputMap();

	// The button map of a context, TNULL for an invalid one
	ButtonMap* GetButtonMap( INPUTCONTEXT a_eContext );

	// The locale string of the first doodad bound to a_eButton in a_pButtonMap, -1 if the button isn't bound
	TINT GetButtonLocaleID( ButtonMap* a_pButtonMap, INPUTBUTTON a_eButton );

private:
	void InitialiseButtonMap();
	void CheckButtonMaps();
	void NamePadDoodads();

private:
	ButtonMap  m_aButtonMaps[ INPUTCONTEXT_NUMOF ]; // 0x04
	NamePadMap m_oNamePadDoodads;                   // 0x11C doodad -> locale string
};

TSTATICASSERT( sizeof( AInputMap ) == 0x138 );
