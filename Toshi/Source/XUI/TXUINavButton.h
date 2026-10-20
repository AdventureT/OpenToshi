#pragma once
#include "TXUIButton.h"

TOSHI_NAMESPACE_START

class XURXUINavButtonData : public XURXUIButtonData
{
public:
	enum Prop
	{
		Prop_PressPath,
		Prop_StayVisible,
		Prop_SrcTransIndex,
		Prop_DestTransIndex,
		Prop_NUMOF,
	};

public:
	XURXUINavButtonData();

	// $deBlob: FUNCTION 006e4210
	// $deBlob: FUNCTION 006e4260
	~XURXUINavButtonData() = default;

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
	TINT32  m_iSrcTransIndex;  // 0x48
	TINT32  m_iDestTransIndex; // 0x4C
	TUINT16 m_uiPressPath;     // 0x50 string index
	TBOOL   m_bStayVisible;    // 0x52
};

//-----------------------------------------------------------------------------
// TXUINavButton (0xA8): a button that, once pressed, creates the first scene of the .xur at PressPath and transitions
// to it (with the source/destination transition indices)
//-----------------------------------------------------------------------------
class TXUINavButton : public TXUIButton
{
public:
	TDECLARE_CLASS( TXUINavButton, TXUIButton );

public:
	TXUINavButton();
	~TXUINavButton();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame ) override;                                       // +0x68
	virtual void  StopAnimation() override;                                                                             // +0x74
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	// NOTE: the base overload is hidden otherwise
	using TXUIElement::PlayAnimation;

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL EndPress() override; // +0xC4

private:
	const TWCHAR* m_wszPressPath;     // 0x98 name of the .xur to go to
	TUINT32       m_uiUnknown9C;      // 0x9C unknown, 0 (StayVisible isn't copied)
	TINT32        m_iSrcTransIndex;   // 0xA0
	TINT32        m_iDestTransIndex;  // 0xA4
};

TOSHI_NAMESPACE_END
