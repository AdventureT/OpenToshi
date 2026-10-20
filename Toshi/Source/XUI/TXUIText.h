#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIFont;

//-----------------------------------------------------------------------------
// XuiText (0x4C bytes)
//-----------------------------------------------------------------------------
class XURXUITextData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_Text,
		Prop_TextColor,
		Prop_DropShadowColor,
		Prop_PointSize,
		Prop_Font,
		Prop_TextStyle,
		Prop_LineSpacingAdjust,
		Prop_NUMOF,
	};

	// Bits of m_uiTextStyle (XUI font styles) used by TXUIText/TXUITextPresenter::Create
	enum TextStyle : TUINT32
	{
		TextStyle_DropShadow     = 0x0001,
		TextStyle_NoWordWrap     = 0x0010,
		TextStyle_JustifyLeft    = 0x0100,
		TextStyle_JustifyCentre2 = 0x0200, // ( style >> 8 ) & 2
		TextStyle_JustifyCentre  = 0x0400,
		TextStyle_VerticalCentre = 0x1000,
	};

public:
	XURXUITextData();

	// $deBlob: FUNCTION 006c3a60
	// $deBlob: FUNCTION 006c2d10
	~XURXUITextData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                        // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;             // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;              // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                  // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                 // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;
	static const TUINT32      sm_uiDefaultFontSize;

public:
	TUINT16 m_uiText;              // 0x30 string index
	TUINT32 m_uiTextColour;        // 0x34
	TUINT32 m_uiDropShadowColour;  // 0x38
	TUINT32 m_uiPointSize;         // 0x3C a float in the XUR
	TUINT16 m_uiFont;              // 0x40 string index
	TUINT32 m_uiTextStyle;         // 0x44 see TextStyle
	TINT32  m_iLineSpacingAdjust;  // 0x48
};

//-----------------------------------------------------------------------------
// TXUIText (0x84): a text drawn with a T2GUIFont ("<font>_<point size>")
//-----------------------------------------------------------------------------
class TXUIText : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUIText, TXUIElement );

public:
	TXUIText();
	~TXUIText();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override; // +0x14

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	// Converts the XUI text style to the T2GUIFont justification
	static TINT GetJustify( TUINT32 a_uiTextStyle )
	{
		if ( a_uiTextStyle & XURXUITextData::TextStyle_JustifyLeft )
			return 0;
		else if ( a_uiTextStyle & XURXUITextData::TextStyle_JustifyCentre )
			return 1;
		else
			return ( a_uiTextStyle >> 8 ) & 2;
	}

	// The texts (and text presenters) clip half a line more at the bottom; AApplication::OnCreate enables it
	// $deBlob: FUNCTION 006e09f0
	static TBOOL IsClipExtended() { return ms_bExtendClip; }

	// NOTE: emitted in game code
	// $deBlob: FUNCTION 005750b0
	static void SetClipExtended( TBOOL a_bExtend ) { ms_bExtendClip = a_bExtend; }

	// NOTE: inlined (ABlobMenu writes the text pointer directly)
	// $deBlob: FUNCTION 0062b010
	void SetText( const TWCHAR* a_wszText ) { m_wszText = a_wszText; }

	// NOTE: inlined (ABlobChallengeDesc writes the colours of its text directly)
	// $deBlob: FUNCTION 00642d70
	void SetColours( const void* a_pIndices, TUINT32* a_pColours )
	{
		m_pColourIndices = a_pIndices;
		m_pColours   = a_pColours;
	}

public:
	static TBOOL ms_bExtendClip;

private:
	T2GUIFont*    m_pFont;              // 0x64
	const TWCHAR* m_wszText;            // 0x68
	const void*   m_pColourIndices;     // 0x6C TUINT8 per character: its colour in m_pColours (T2GUIFont::DrawTextBoxMultiColoured)
	TUINT32*      m_pColours;           // 0x70 if set, the text is drawn multi coloured
	TINT          m_iJustify;           // 0x74 T2GUIFont justification, see GetJustify
	TUINT32       m_uiDropShadowColour; // 0x78
	TINT32        m_iLineSpacingAdjust; // 0x7C
	TBOOL         m_bDropShadow;        // 0x80
	TBOOL         m_bWordWrap;          // 0x81
	TBOOL         m_bVerticalCentre;    // 0x82
	TBOOL         m_bReleaseFont;       // 0x83 TTRUE by default, releases m_pFont in the destructor
};

TOSHI_NAMESPACE_END
