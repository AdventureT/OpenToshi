#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

class TXUIVisual;
class TXUIScene;

class XURXUIControlData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_ClassOverride,
		Prop_Visual,
		Prop_Enabled,
		Prop_UnfocussedInput,
		Prop_NavLeft,
		Prop_NavRight,
		Prop_NavUp,
		Prop_NavDown,
		Prop_Text,
		Prop_Unused9,
		Prop_ImagePath,
		Prop_NUMOF,
	};

public:
	XURXUIControlData();
	// $deBlob: FUNCTION 006cb740
	~XURXUIControlData() = default;

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
	TUINT16 m_uiClassOverride;   // 0x30
	TUINT16 m_uiVisual;          // 0x32
	TUINT16 m_uiNavLeft;         // 0x34
	TUINT16 m_uiNavRight;        // 0x36
	TUINT16 m_uiNavUp;           // 0x38
	TUINT16 m_uiNavDown;         // 0x3A
	TUINT16 m_uiText;            // 0x3C
	TUINT16 m_uiImagePath;       // 0x3E
	TBOOL   m_bEnabled;          // 0x40
	TBOOL   m_bUnfocussedInput;  // 0x41
};

//-----------------------------------------------------------------------------
// TXUIControl (0x94): an element with a visual (a TXUIVisual from the skin), text, image path and navigation
//-----------------------------------------------------------------------------
class TXUIControl : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUIControl, TXUIElement );

public:
	enum Nav
	{
		Nav_Left,
		Nav_Right,
		Nav_Up,
		Nav_Down,
		Nav_NUMOF,
	};

	// m_uiControlFlags
	enum ControlFlags : TUINT8
	{
		ControlFlags_Enabled         = BITFLAG( 0 ),
		ControlFlags_Checkable       = BITFLAG( 1 ),
		ControlFlags_IgnoreInput     = BITFLAG( 2 ), // set while its scene is in a transition
		ControlFlags_UnfocussedInput = BITFLAG( 3 ),
	};

	// Visual states played by PlayVisual (named frame pairs of the XUI visual timelines)
	enum VisualFrame
	{
		VisualFrame_Normal         = 0,
		VisualFrame_Disabled       = 2,
		VisualFrame_NormalFocus    = 4,
		VisualFrame_DisabledFocus  = 7,
		VisualFrame_InitFocus      = 10,
		VisualFrame_Press          = 15,
		VisualFrame_NormalChecked  = 21,
		VisualFrame_DisabledChecked = 28,
	};

public:
	TXUIControl();
	~TXUIControl();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override;                                           // +0x14
	virtual void SetWidth( TFLOAT a_fWidth ) override;                        // +0x28
	virtual void SetHeight( TFLOAT a_fHeight ) override;                      // +0x2C
	virtual void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override; // +0x30

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual TBOOL SendMessage( TINT a_eMessage, const TWCHAR* a_wszParam ) override;                                 // +0x78
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
	virtual TBOOL IsCheckable() override;                                     // +0xA0
	virtual TBOOL IsEnabled() override;                                       // +0xA4
	virtual void  ResolveNavigation( TXUIElement* a_pRoot ) override;         // +0xAC

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL         Press();                                                     // +0xC0
	virtual TBOOL         EndPress();                                                  // +0xC4
	virtual TBOOL         Focus();                                                     // +0xC8
	virtual TBOOL         NavLeft();                                                   // +0xCC
	virtual TBOOL         NavRight();                                                  // +0xD0
	virtual TBOOL         NavUp();                                                     // +0xD4
	virtual TBOOL         NavDown();                                                   // +0xD8
	virtual TBOOL         NotifyValueChanged();                                        // +0xDC
	virtual TBOOL         Back();                                                      // +0xE0
	virtual const TWCHAR* GetText();                                                   // +0xE4
	virtual void          SetText( const TWCHAR* a_wszText, TINT a_iParam );           // +0xE8
	virtual void          SetText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex );      // +0xEC
	virtual const TWCHAR* GetImagePath();                                              // +0xF0
	virtual void          SetImagePath( const TWCHAR* a_wszImagePath );                // +0xF4
	virtual TBOOL         HandleButton( TINT a_eButton, TINT a_iController );          // +0xF8
	virtual TBOOL         HandleButtonRelease( TINT a_eButton, TINT a_iController );   // +0xFC
	virtual TBOOL         HandleAnalog( TINT a_iParam1, TINT a_iParam2, TINT a_iController );            // +0x100
	virtual void          CreateTimelineControl( TXUIResource& a_rResource, XURXUIObjectData* a_pData ); // +0x104
	virtual void          SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation );     // +0x108
	virtual TBOOL         AcceptsUnfocussedInput();                                                      // +0x10C

	void CreateVisual( TXUIResource& a_rResource, const TWCHAR* a_wszVisual );
	void CreateVisual( TXUIResource& a_rResource, TUINT32 a_uiVisualIndex );
	void PlayVisual( TBOOL a_bNoAnimation, TBOOL a_bFromNavigation, TBOOL a_bInit );
	void SetUnfocussedInput( TBOOL a_bUnfocussedInput );

	// $deBlob: FUNCTION 0062d0b0
	void SetEnabled( TBOOL a_bEnabled )
	{
		if ( a_bEnabled )
		{
			if ( m_uiControlFlags & ControlFlags_Enabled ) return;
			m_uiControlFlags |= ControlFlags_Enabled;
		}
		else
		{
			if ( !( m_uiControlFlags & ControlFlags_Enabled ) ) return;
			m_uiControlFlags &= ~ControlFlags_Enabled;
		}

		PlayVisual( TFALSE, TFALSE, TFALSE );
	}

	TXUIScene* FindParentScene();

	TXUIControl* GetNav( Nav a_eNav ) const { return m_apNav[ a_eNav ]; }
	void         SetNav( Nav a_eNav, TXUIControl* a_pControl ) { m_apNav[ a_eNav ] = a_pControl; }
	void         SetNavId( Nav a_eNav, const TWCHAR* a_wszId ) { m_awszNav[ a_eNav ] = a_wszId; }

	// $deBlob: FUNCTION 00580550
	TXUIVisual* GetVisual() const { return m_pVisual; }

	// NOTE: inlined (the game sets the text pointer without updating the visual)
	void SetTextPointer( const TWCHAR* a_wszText ) { m_wszText = a_wszText; }

	// Focused in the focus state and in the tab transition of a TXUITabScene
	// $deBlob: FUNCTION 00637600
	TBOOL IsFocused() const
	{
		TUINT32 uiState = GetTimelineState();
		return uiState == TimelineState_Focus || uiState == TimelineState_TransTab;
	}

	// $deBlob: FUNCTION 006be740
	TBOOL IsInputIgnored() const { return ( m_uiControlFlags >> 2 ) & 1; }
	void  SetInputIgnored( TBOOL a_bIgnored )
	{
		if ( a_bIgnored ) m_uiControlFlags |= ControlFlags_IgnoreInput;
		else m_uiControlFlags &= ~ControlFlags_IgnoreInput;
	}

	// $deBlob: FUNCTION 006cb690
	static TXUIControl* CastFromTObject( TObject* a_pObject )
	{
		TASSERT( ( a_pObject == TNULL ) || a_pObject->IsA( &TGetClass( TXUIControl ) ) );
		return a_pObject ? TSTATICCAST( TXUIControl, a_pObject ) : TNULL;
	}

protected:
	TXUIVisual*   m_pVisual;                 // 0x64
	const TWCHAR* m_wszText;                 // 0x68
	const TWCHAR* m_wszImagePath;            // 0x6C
	const TWCHAR* m_awszNav[ Nav_NUMOF ];    // 0x70 Ids of the controls to navigate to
	TXUIControl*  m_apNav[ Nav_NUMOF ];      // 0x80 resolved by ResolveNavigation
	TUINT8        m_uiControlFlags;          // 0x90
	TINT8         m_iPressController;        // 0x91 -1 if not pressed
};

TOSHI_NAMESPACE_END
