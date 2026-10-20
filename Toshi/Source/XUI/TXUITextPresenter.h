#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIFont;

//-----------------------------------------------------------------------------
// XuiTextPresenter (0x4C bytes): the text of a control's visual
//-----------------------------------------------------------------------------
class XURXUITextPresenterData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_TextColor,
		Prop_DropShadowColor,
		Prop_PointSize,
		Prop_Font,
		Prop_TextStyle,
		Prop_LineSpacingAdjust,
		Prop_DataAssociation,
		Prop_NUMOF,
	};

public:
	XURXUITextPresenterData();

	// $deBlob: FUNCTION 006e09c0
	// $deBlob: FUNCTION 006e0960
	~XURXUITextPresenterData() = default;

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

public:
	TUINT32 m_uiDataAssociation;  // 0x30 index of the control's text it presents
	TUINT32 m_uiTextColour;       // 0x34
	TUINT32 m_uiDropShadowColour; // 0x38
	TUINT32 m_uiPointSize;        // 0x3C a float in the XUR
	TUINT16 m_uiFont;             // 0x40 string index
	TUINT32 m_uiTextStyle;        // 0x44 see XURXUITextData::TextStyle
	TINT32  m_iLineSpacingAdjust; // 0x48
};

//-----------------------------------------------------------------------------
// TXUITextPresenter (0x8C): draws the text a TXUIVisual/TXUIGroup gives it (SetPresenterData/SetPresenterText)
//-----------------------------------------------------------------------------
class TXUITextPresenter : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUITextPresenter, TXUIElement );

public:
	TXUITextPresenter();
	~TXUITextPresenter();

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

	TUINT32 GetDataAssociation() const { return m_uiDataAssociation; }

	// $deBlob: FUNCTION 006cf3f0
	const TWCHAR* GetText() const { return m_wszText; }

	// Inlined by TXUIVisual/TXUIGroup
	void SetText( const TWCHAR* a_wszText, TINT a_iParam )
	{
		m_iParam  = a_iParam;
		m_wszText = a_wszText;
	}

	// Inlined by TXUIVisual/TXUIGroup::SetPresenterText2
	// $deBlob: FUNCTION 006d5010
	void SetMultiColour( const void* a_pColourIndices, TUINT32* a_pColours )
	{
		m_pColourIndices = a_pColourIndices;
		m_pColours   = a_pColours;
	}

private:
	T2GUIFont*    m_pFont;              // 0x64
	const TWCHAR* m_wszText;            // 0x68
	const void*   m_pColourIndices;     // 0x6C TUINT8 per character: its colour in m_pColours (T2GUIFont::DrawTextBoxMultiColoured)
	TUINT32*      m_pColours;           // 0x70 if set, the text is drawn multi coloured
	TINT          m_iJustify;           // 0x74 T2GUIFont justification, see TXUIText::GetJustify
	TUINT32       m_uiDropShadowColour; // 0x78
	TINT32        m_iLineSpacingAdjust; // 0x7C
	TUINT32       m_uiDataAssociation;  // 0x80
	TINT          m_iParam;             // 0x84 the number of characters drawn (T2GUIFont::DrawTextBox), -1: all
	TBOOL         m_bDropShadow;        // 0x88
	TBOOL         m_bWordWrap;          // 0x89
	TBOOL         m_bVerticalCentre;    // 0x8A
};

TOSHI_NAMESPACE_END
