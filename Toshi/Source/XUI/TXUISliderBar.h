#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class TXUITimelineControl;

class XURXUISliderBarData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_RangeMin,
		Prop_RangeMax,
		Prop_Value,
		Prop_Step,
		Prop_Vertical,
		Prop_AccelInc,
		Prop_AccelTime,
		Prop_NUMOF,
	};

public:
	XURXUISliderBarData();

	// $deBlob: FUNCTION 006e64f0
	// $deBlob: FUNCTION 006e6560
	~XURXUISliderBarData() = default;

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
	TINT32 m_iRangeMin;  // 0x44
	TINT32 m_iRangeMax;  // 0x48
	TINT32 m_iValue;     // 0x4C
	TINT32 m_iStep;      // 0x50
	TINT32 m_iAccelInc;  // 0x54
	TINT32 m_iAccelTime; // 0x58
	TBOOL  m_bVertical;  // 0x5C
};

//-----------------------------------------------------------------------------
// TXUISliderBar (0x2C8): a value changed with the left/right (up/down if vertical) buttons or dragged with the mouse;
// shows the value as text ("%d") and moves the timeline of the visual's "SliderBody" group
//-----------------------------------------------------------------------------
class TXUISliderBar : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUISliderBar, TXUIControl );

public:
	TXUISliderBar();
	~TXUISliderBar();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                      // +0xF8
	virtual TBOOL HandleButtonRelease( TINT a_eButton, TINT a_iController ) override;               // +0xFC
	virtual TBOOL HandleAnalog( TINT a_iParam1, TINT a_iParam2, TINT a_iController ) override;      // +0x100 the mouse position
	virtual void  SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation ) override; // +0x108

	//-----------------------------------------------------------------------------
	// TXUISliderBar
	//-----------------------------------------------------------------------------
	virtual TINT GetValue();                // +0x110
	virtual void SetValue( TINT a_iValue ); // +0x114
	virtual void UpdateText();              // +0x118

	TINT GetRangeMin() const { return m_iRangeMin; } // 0x298
	TINT GetRangeMax() const { return m_iRangeMax; } // 0x29C

	void UpdateSliderPosition();

private:
	TWCHAR               m_wszValue[ 256 ];    // 0x094 the value as text, shown by the visual
	TINT                 m_iValue;             // 0x294
	TINT                 m_iRangeMin;          // 0x298
	TINT                 m_iRangeMax;          // 0x29C
	TINT                 m_iStep;              // 0x2A0
	TXUITimelineControl* m_pBodyTimeline;      // 0x2A4 the timeline control of m_pSliderBody
	TFLOAT               m_fNormalTime;        // 0x2A8 time of its "Normal" named frame
	TFLOAT               m_fFocusTime;         // 0x2AC time of its "Focus" named frame
	TBOOL                m_bVertical;          // 0x2B0
	TINT                 m_iAccelInc;          // 0x2B4 only set by the timelines, unused
	TINT                 m_iAccelTime;         // 0x2B8 only set by the timelines, unused
	TBOOL                m_bCapturing;         // 0x2BC dragged with the mouse (TXUI's capturing control)
	TXUIElement*         m_pSliderBody;        // 0x2C0 the "SliderBody" group of the visual
	T2GUIElement*        m_pSliderBodyChild;   // 0x2C4 its first child
};

TOSHI_NAMESPACE_END
