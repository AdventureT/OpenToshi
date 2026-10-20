#pragma once
#include "Entities/AEntityComponent2.h"

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: Blob's paint colour (created by ABlobEntity::Create at 0x3C8): the start colour, the current colour (paint
// mixes into it, COLOUR_BLACK is ink), the fading colour pair of ABlobColourMaps' interpolation map and the rainbow
// colour (BFLAG 0x200000).
// NOTE: the class name
//-----------------------------------------------------------------------------
class ABlobColourComponent
    : public AEntityComponent2
{
	friend class ABlobEntity; // the paint spray reads m_uiFadeColour2
	friend class ABlobRibbonTrailComponent;

public:
	enum COLOUR : TUINT32
	{
		COLOUR_WHITE = 0,
		COLOUR_RED   = 4,    // (ALeechBot2 assert)
		COLOUR_BLACK = 0x10, // ink
		COLOUR_NUMOF = 0x11,
	};

public:
	ABlobColourComponent( ABlobEntity* a_pOwner );
	~ABlobColourComponent();

	virtual void Tick( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x04

	// Back to the start colour
	void Reset();

	// The colour a_uiColour mixes to with the current colour
	void GetMixedColour( TUINT32& a_rOutColour, TUINT32 a_uiColour ) const;

	// Mixes a_uiColour into the current colour / replaces it
	void MixColour( TUINT32 a_uiColour );
	void SetColour( TUINT32 a_uiColour );

	TUINT32 GetCurrentColour() const { return m_uiCurrentColour; }
	// $deBlob: FUNCTION 005acfc0
	TUINT32 GetStartColour() const { return m_uiStartColour; }
	// $deBlob: FUNCTION 00578660
	TUINT32 GetUnk20() const { return m_uiUnk20; }
	// $deBlob: FUNCTION 005ad000
	void SetStartColour( TUINT32 a_uiColour ) { m_uiStartColour = a_uiColour; }
	// $deBlob: FUNCTION 005ad010
	void    SetUnk20( TUINT32 a_uiColour ) { m_uiUnk20 = a_uiColour; }
	TUINT32 GetFadeColour() const { return m_uiFadeColour; }
	TUINT32 GetFadeColour2() const { return m_uiFadeColour2; }

	// a_uiNewColour mixed into a_uiColour (primary colours < 8 mix, the others replace)
	static TUINT32 MixColours( TUINT32 a_uiColour, TUINT32 a_uiNewColour );

	// Per byte lerp from a_uiFrom to a_uiTo
	static TUINT32 LerpColour( TUINT32 a_uiFrom, TUINT32 a_uiTo, TFLOAT a_fT );

	// The colour at a_fT between the two colours of the interpolation map entry a_iIndex
	static TUINT32 GetInterpolatedColour( TINT a_iIndex, TFLOAT a_fT );

	static TUINT32* GetInterpolationColours( TINT a_iIndex );

	// A random primary or secondary colour (1-7)
	static TINT GetRandomColour();

	static TUINT32 GetComplementaryColour( TUINT32 a_uiColour );

public:
	static const TUINT32 ms_auiColours[ COLOUR_NUMOF ];

	static const TUINT32 ms_auiDarkColours[ COLOUR_NUMOF ];

private:
	TFLOAT       m_fRainbowHue;         // 0x14 +90 degrees/s
	TFLOAT       m_fRainbowHue2;        // 0x18 +180 degrees/s, the one used
	TUINT32      m_uiStartColour;       // 0x1C
	TUINT32      m_uiUnk20;             // 0x20
	TUINT32      m_uiCurrentColour;     // 0x24
	TUINT32      m_uiFadeColour;        // 0x28
	TUINT32      m_uiFadeColour2;       // 0x2C the reverse fade
	TUINT8       m_aRainbowColour[ 4 ]; // 0x30 b, g, r
	TFLOAT       m_fFade;               // 0x34 0..1
	TBOOL        m_bFadeUp;             // 0x38
	ABlobEntity* m_pOwner;              // 0x3C
};                                      // 0x40
