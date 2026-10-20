#pragma once
#include "TXUIScene.h"

TOSHI_NAMESPACE_START

class XURXUITabSceneData : public XURXUISceneData
{
public:
	enum Prop
	{
		Prop_TabCount,
		Prop_Wrap,
		Prop_UserInterrupt,
		Prop_VerticalTabs,
		Prop_NoAutoHide,
		Prop_DefaultTab,
		Prop_NUMOF,
	};

public:
	XURXUITabSceneData();

	// $deBlob: FUNCTION 006e8530
	// $deBlob: FUNCTION 006e8580
	~XURXUITabSceneData() = default;

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
	TUINT32 m_uiTabCount;     // 0x54
	TUINT32 m_uiDefaultTab;   // 0x58
	TBOOL   m_bWrap;          // 0x5C
	TBOOL   m_bUserInterrupt; // 0x5D
	TBOOL   m_bVerticalTabs;  // 0x5E
	TBOOL   m_bNoAutoHide;    // 0x5F
};

//-----------------------------------------------------------------------------
// TXUITabScene (0x140): a scene showing one of its child scenes ("Tab1", "Tab2"...), left/right switch the tab and
// play the "<from>To<to>" named frames
//-----------------------------------------------------------------------------
class TXUITabScene : public TXUIScene
{
public:
	TDECLARE_CLASS( TXUITabScene, TXUIScene );

public:
	TXUITabScene();
	~TXUITabScene();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL        Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual TXUIElement* RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController ) override;  // +0x64
	virtual TBOOL        SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override;                                    // +0x78
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL NotifyValueChanged() override;                               // +0xDC
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	// TXUITabScene.h:81
	// $deBlob: FUNCTION 0063a400
	TXUIScene* GetTab( TINT a_iTab ) const
	{
		TASSERT( a_iTab >= 0 && a_iTab < m_uiNumTabs );
		return m_apTabs[ a_iTab ];
	}

	// $deBlob: FUNCTION 0063a3d0
	TUINT32 GetNumTabs() const { return m_uiNumTabs; }
	// $deBlob: FUNCTION 0063a3c0
	TUINT32 GetCurrentTab() const { return m_uiCurrentTab; }
	// $deBlob: FUNCTION 0063a3b0
	void SetForwardButtons( TBOOL a_bForward ) { m_bForwardButtons = a_bForward; }

public:
	// The end named frame of the current tab transition ("<from>To<to>End"), SendMessage ends the transition on it
	static TWCHAR ms_wszTransTabEnd[ 256 ];

protected:
	TXUIScene** m_apTabs;        // 0x130
	TUINT32     m_uiNumTabs;     // 0x134
	TUINT32     m_uiCurrentTab;  // 0x138
	TBOOL       m_bVerticalTabs; // 0x13C left/right don't switch the tabs
	TBOOL       m_bWrap;         // 0x13D
	TBOOL       m_bForwardButtons; // 0x13E unknown, forwards the buttons to the current tab when set (ABlobOptionsMenu sets it)
};

TOSHI_NAMESPACE_END
