#pragma once
#include "TXUIControl.h"

TOSHI_NAMESPACE_START

class TXUITimelineControl;

class XURXUIProgressBarData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_RangeMin,
		Prop_RangeMax,
		Prop_Value,
		Prop_NUMOF,
	};

public:
	XURXUIProgressBarData();

	// $deBlob: FUNCTION 006e4b60
	// $deBlob: FUNCTION 006e4bb0
	~XURXUIProgressBarData() = default;

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
	TINT32 m_iRangeMin; // 0x44
	TINT32 m_iRangeMax; // 0x48
	TINT32 m_iValue;    // 0x4C
};

//-----------------------------------------------------------------------------
// TXUIProgressBar (0x2B0): shows its value as text ("%d") and moves the timeline of the visual's "ProgressBody"
// group between its start and its end
//-----------------------------------------------------------------------------
class TXUIProgressBar : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIProgressBar, TXUIControl );

public:
	TXUIProgressBar();
	~TXUIProgressBar();

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
	// TXUIProgressBar
	//-----------------------------------------------------------------------------
	virtual TINT GetValue();                       // +0x110
	virtual void SetValue( TINT a_iValue );        // +0x114
	virtual TINT GetRangeMin();                    // +0x118
	virtual void SetRangeMin( TINT a_iRangeMin );  // +0x11C
	virtual TINT GetRangeMax();                    // +0x120
	virtual void SetRangeMax( TINT a_iRangeMax );  // +0x124

private:
	TWCHAR               m_wszValue[ 256 ];      // 0x094 the value as text, shown by the visual
	TINT                 m_iValue;               // 0x294
	TINT                 m_iRangeMin;            // 0x298
	TINT                 m_iRangeMax;            // 0x29C
	TXUITimelineControl* m_pBodyTimeline;        // 0x2A0 the timeline control of m_pProgressBody
	TFLOAT               m_fStartTime;           // 0x2A4 the time of m_pBodyTimeline when created
	TXUIElement*         m_pProgressBody;        // 0x2A8 the "ProgressBody" group of the visual
	T2GUIElement*        m_pProgressBodyChild;   // 0x2AC its first child
};

TOSHI_NAMESPACE_END
