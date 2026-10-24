#pragma once
#include "TXUIElement.h"
#include "TXUITimelineControl.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// XuiGroup: no properties of its own (0x30 bytes)
//-----------------------------------------------------------------------------
class XURXUIGroupData : public XURXUIElementData
{
public:
	XURXUIGroupData();

	// $deBlob: FUNCTION 006d4fe0
	// $deBlob: FUNCTION 006d4f80
	~XURXUIGroupData() = default;

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
};

//-----------------------------------------------------------------------------
// TXUIGroup (0xB4): a group of elements with its own timelines; forwards the presenter data (text, image) to its
// TXUITextPresenter/TXUIImagePresenter children like TXUIVisual (but also hides the ones that don't match)
//-----------------------------------------------------------------------------
class TXUIGroup : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUIGroup, TXUIElement );

public:
	TXUIGroup();
	~TXUIGroup();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame ) override;                                       // +0x68
	virtual void  PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame ) override;                 // +0x6C
	virtual void  PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd ) override;                                         // +0x70
	virtual void  StopAnimation() override;                                                                             // +0x74
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
	virtual void  SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath, TUINT32 a_uiTextIndex, TUINT32 a_uiImageIndex, TINT a_iParam ) override; // +0x84
	virtual void  SetPresenterText( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam ) override;  // +0x88
	virtual void  SetPresenterText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam ) override; // +0x8C
	virtual void  SetPresenterImagePath( const TWCHAR* a_wszImagePath, TUINT32 a_uiIndex ) override;       // +0x90
	virtual void  ResolveNavigation( TXUIElement* a_pRoot ) override;                                     // +0xAC

	TXUITimelineControl* GetTimelineControl() { return &m_oTimelineControl; }

private:
	TXUITimelineControl m_oTimelineControl; // 0x64
};

TOSHI_NAMESPACE_END
