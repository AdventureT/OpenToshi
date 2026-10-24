#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class XURXUIEditData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_TextLimit,
		Prop_AllowedChars,
		Prop_PasswordChar,
		Prop_ReadOnly,
		Prop_Multiline,
		Prop_SmoothScroll,
		Prop_NUMOF,
	};

public:
	XURXUIEditData();

	// $deBlob: FUNCTION 006e3310
	// $deBlob: FUNCTION 006e3360
	~XURXUIEditData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override;
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;
	virtual const TXUITypeInfo* GetTypeInfo() const override;

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	// NOTE: retail's constructor doesn't initialise them
	TUINT32 m_uiTextLimit;    // 0x44
	TUINT16 m_uiAllowedChars; // 0x48 string index
	TUINT16 m_uiPasswordChar; // 0x4A
	TUINT8  m_uiReadOnly;     // 0x4C
	TUINT8  m_uiMultiline;    // 0x4D
	TUINT8  m_uiSmoothScroll; // 0x4E
};

//-----------------------------------------------------------------------------
// TXUIEditBox (0x94): an edit control; de Blob doesn't implement editing, it's a plain control
//-----------------------------------------------------------------------------
class TXUIEditBox : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIEditBox, TXUIControl );

public:
	TXUIEditBox();
	~TXUIEditBox();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
};

TOSHI_NAMESPACE_END
