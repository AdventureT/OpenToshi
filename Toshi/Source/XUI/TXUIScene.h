#pragma once
#include "TXUIControl.h"
#include "TXUITimelineControl.h"

#include <functional>

TOSHI_NAMESPACE_START

class TXUIBackButton;
class TXUIVisual;

class XURXUISceneData : public XURXUIControlData
{
public:
	enum Prop
	{
		Prop_DefaultFocus,
		Prop_TransFrom,
		Prop_TransTo,
		Prop_TransBackFrom,
		Prop_TransBackTo,
		Prop_InterruptTransitions,
		Prop_IgnorePresses,
		Prop_NUMOF,
	};

public:
	XURXUISceneData();

	// $deBlob: FUNCTION 006cf250
	// $deBlob: FUNCTION 006cf350
	~XURXUISceneData() = default;

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
	TUINT32 m_uiInterruptTransitions; // 0x44
	TUINT16 m_uiDefaultFocus;         // 0x48
	TUINT16 m_uiTransFrom;            // 0x4A
	TUINT16 m_uiTransTo;              // 0x4C
	TUINT16 m_uiTransBackFrom;        // 0x4E
	TUINT16 m_uiTransBackTo;          // 0x50
	TBOOL   m_bIgnorePresses;         // 0x52
};

//-----------------------------------------------------------------------------
// TXUIScene (0x130): a control with focus per controller, transitions (TransTo/From, TransBackTo/From visuals and
// named frames) and its own timelines.
// de Blob's PC port added the platform button glyphs to this file: the scenes show the elements of the current
// platform (Wii, Xbox 360, keyboard, PS4) and the glyph characters of the strings are remapped to its buttons.
//-----------------------------------------------------------------------------
class TXUIScene : public TXUIControl
{
public:
	TDECLARE_CLASS( TXUIScene, TXUIControl );

public:
	static TCONSTEXPR_VAR TINT MAXCONTROLLERS = 4; // TXUIInput::MAXCONTROLLERS

	// Scene states (bits 6-9 of m_uiXUIState)
	enum State : TUINT32
	{
		State_None          = 0,
		State_Focus         = 2,
		State_Press         = 3,
		State_TransFrom     = 4,
		State_TransTo       = 5,
		State_TransBackFrom = 6,
		State_TransBackTo   = 7,
		State_Hidden        = 8,
		State_TransTab      = 9, // TXUITabScene
	};

	// m_uiSceneFlags (+0x95)
	enum SceneFlags : TUINT8
	{
		SceneFlags_Delete             = BITFLAG( 0 ), // deleted (or unlinked if cached) by the next Tick
		SceneFlags_Cached             = BITFLAG( 1 ),
		SceneFlags_TransitionEnded    = BITFLAG( 2 ), // the End* named frame of the transition was reached
		SceneFlags_HideAfterTransFrom = BITFLAG( 3 ),
		SceneFlags_AcceptsInput       = BITFLAG( 4 ),
		SceneFlags_AnyController      = BITFLAG( 5 ),
	};

	// The transition visuals (SetTransitionVisual), in the order of the XURXUISceneData properties
	enum Transition
	{
		Transition_From,
		Transition_To,
		Transition_BackFrom,
		Transition_BackTo,
	};

	//-----------------------------------------------------------------------------
	// Platform glyphs (de Blob PC port)
	//-----------------------------------------------------------------------------
	enum Platform
	{
		Platform_Wii,
		Platform_Xbox360,
		Platform_Keyboard,
		Platform_PS4,
		Platform_NUMOF,
	};

	static TCONSTEXPR_VAR TINT NUM_PLATFORM_GLYPHS = 21;

	// A binding of the game's controls (ABlobInputMap_Win.cpp: the setup.ini key name and the doodad/button), the
	// glyphs are built from the button of each binding
	struct InputBinding
	{
		const TCHAR* m_szName;  // 0x0
		TINT         m_iButton; // 0x4
	};

	// NOTE: retail reads the game's bindings (keyboard, Xbox 360 and PS4, NUM_PLATFORM_GLYPHS
	// each) directly; here the game installs them

public:
	TXUIScene();
	~TXUIScene();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL        Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual TXUIElement* RayTraceDeepest( const TVector2& a_rPoint, const TVector2& a_rScale, TINT a_iController ) override;  // +0x64
	virtual void         PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame ) override;                                       // +0x68
	virtual void         PlayAnimation( const TWCHAR* a_wszStartFrame, const TWCHAR* a_wszEndFrame ) override;                 // +0x6C
	virtual void         PlayTimelineRange( TFLOAT a_fStart, TFLOAT a_fEnd ) override;                                         // +0x70
	virtual void         StopAnimation() override;                                                                             // +0x74
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void         UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80
	virtual void         SetPresenterData( const TWCHAR* a_wszText, const TWCHAR* a_wszImagePath, TUINT32 a_uiTextIndex, TUINT32 a_uiImageIndex, TINT a_iParam ) override; // +0x84
	virtual void         SetPresenterText( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam ) override;  // +0x88
	virtual void         SetPresenterText2( const TWCHAR* a_wszText, TUINT32 a_uiIndex, TINT a_iParam ) override; // +0x8C
	virtual void         SetPresenterImagePath( const TWCHAR* a_wszImagePath, TUINT32 a_uiIndex ) override;       // +0x90
	virtual void         OnCreated() override;                                                                     // +0x94

	//-----------------------------------------------------------------------------
	// TXUIControl
	// NOTE: messages 4-7 (+0xCC..+0xD8) are sent by the End* named frames of the transitions (see
	// TXUITimelineControl::SendMessage): 4 EndTransFrom, 5 EndTransTo, 6 EndTransBackFrom, 7 EndTransBackTo
	//-----------------------------------------------------------------------------

	// $deBlob: FUNCTION 00625e00
	virtual TBOOL NavLeft() override // +0xCC
	{
		m_uiSceneFlags |= SceneFlags_TransitionEnded;
		return TTRUE;
	}

	// $deBlob: FUNCTION 00625e10
	virtual TBOOL NavRight() override // +0xD0
	{
		m_uiSceneFlags |= SceneFlags_TransitionEnded;
		return TTRUE;
	}

	// $deBlob: FUNCTION 00625de0
	virtual TBOOL NavUp() override // +0xD4
	{
		m_uiSceneFlags |= SceneFlags_TransitionEnded;
		return TTRUE;
	}

	// $deBlob: FUNCTION 00625df0
	virtual TBOOL NavDown() override // +0xD8
	{
		m_uiSceneFlags |= SceneFlags_TransitionEnded;
		return TTRUE;
	}

	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                            // +0xF8
	virtual void  CreateTimelineControl( TXUIResource& a_rResource, XURXUIObjectData* a_pData ) override; // +0x104
	virtual void  SetFocus( TBOOL a_bFocus, TBOOL a_bNoAnimation, TBOOL a_bFromNavigation ) override;   // +0x108

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void OnMouseOverChange( TXUIElement* a_pElement, TINT a_iController ); // +0x110
	virtual void TransTo();                                                        // +0x114
	virtual void TransFrom();                                                      // +0x118
	virtual void TransBackTo();                                                    // +0x11C
	virtual void TransBackFrom();                                                  // +0x120
	virtual void EndTransTo();                                                     // +0x124
	virtual void EndTransFrom();                                                   // +0x128
	virtual void EndTransBackTo();                                                 // +0x12C
	virtual void EndTransBackFrom();                                               // +0x130

	void   ResetAnimation();
	void   SetCached();
	void   SetTransitionVisual( Transition a_eTransition, const TWCHAR* a_wszName );
	TFLOAT GetTimelineTime( const TWCHAR* a_wszNamedFrame );
	void   SetTimelineTime( TFLOAT a_fTime );

	// $deBlob: FUNCTION 006d9bf0
	static TXUIScene* CastFromTObject( TObject* a_pObject )
	{
		TASSERT( ( a_pObject == TNULL ) || a_pObject->IsA( &TGetClass( TXUIScene ) ) );
		return a_pObject ? TSTATICCAST( TXUIScene, a_pObject ) : TNULL;
	}

	// TXUIScene.h:152
	// $deBlob: FUNCTION 005a2a60
	TXUIControl* GetFocus( TINT a_iController ) const
	{
		TASSERT( a_iController >= 0 && a_iController < TXUIScene::MAXCONTROLLERS );
		return m_apFocus[ a_iController ];
	}

	// TXUIScene.h:164
	// $deBlob: FUNCTION 005c8a10
	void SetFocus( TXUIControl* a_pControl, TINT a_iController )
	{
		TASSERT( a_iController >= 0 && a_iController < TXUIScene::MAXCONTROLLERS );
		m_apFocus[ a_iController ] = a_pControl;
	}

	// The default focus (m_pDefaultFocus, 0x108): the options menu clears it on its tabs
	// $deBlob: FUNCTION 0063a4a0
	void SetDefaultFocus( TXUIControl* a_pControl ) { m_pDefaultFocus = a_pControl; }

	using TXUIControl::SetFocus;

	TXUIScene* GetPrevScene() const { return m_pPrevScene; }
	void       SetPrevScene( TXUIScene* a_pScene ) { m_pPrevScene = a_pScene; }

	TBOOL AcceptsAnyController() const { return ( m_uiSceneFlags & SceneFlags_AnyController ) != 0; }

	// NOTE: inlined (the Blob Party engagement menu takes the input of every controller)
	// $deBlob: FUNCTION 0063ed00
	void SetAcceptsAnyController( TBOOL a_bAny )
	{
		if ( a_bAny )
			m_uiSceneFlags |= SceneFlags_AnyController;
		else
			m_uiSceneFlags &= ~SceneFlags_AnyController;
	}

	// The scene is deleted (or unlinked if cached) by the next Tick
	// $deBlob: FUNCTION 00638d20
	void MarkForDelete() { m_uiSceneFlags |= SceneFlags_Delete; }

	TBOOL IsAcceptingInput() const { return ( m_uiSceneFlags & SceneFlags_AcceptsInput ) != 0; }

	void SetAcceptsInput( TBOOL a_bAccepts )
	{
		if ( a_bAccepts ) m_uiSceneFlags |= SceneFlags_AcceptsInput;
		else m_uiSceneFlags &= ~SceneFlags_AcceptsInput;
	}

	TXUITimelineControl& GetTimelineControl() { return m_oTimelineControl; }

	TINT GetTransitionIndex() const { return m_iTransitionIndex; }
	// $deBlob: FUNCTION 005b9f00
	void SetTransitionIndex( TINT a_iIndex ) { m_iTransitionIndex = a_iIndex; }

	//-----------------------------------------------------------------------------
	// Platform glyphs (de Blob PC port)
	//-----------------------------------------------------------------------------
	static TBOOL        ForEachElement( TXUIElement* a_pElement, TBOOL a_bVisuals, const std::function<TBOOL( TXUIElement* )>& a_rfnCallback );
	static TXUIElement* FindElement( TXUIElement* a_pRoot, const TWCHAR* a_wszName, TBOOL a_bCheckRoot );
	static void         UpdatePlatformElements( TXUIElement* a_pRoot, TBOOL a_bTick );
	static TWCHAR*      ReplaceString( const TWCHAR* a_wszString, const TWCHAR* a_wszFind, const TWCHAR* a_wszReplace, TWCHAR* a_wszOutString );
	static TBOOL        IsPlatformGlyph( TWCHAR a_wcChar );
	static TBOOL        IsInputGlyph( TWCHAR a_wcChar );
	static TWCHAR       GetKeyboardGlyph( TINT a_iKey );
	static TWCHAR       GetXbox360Glyph( TINT a_iButton );
	static TWCHAR       GetPS4Glyph( TINT a_iButton );
	static void         InitPlatformGlyphs();
	static void         FixMovementGlyphs( TWCHAR* a_wszString );
	static TWCHAR*      RemapPlatformGlyphs( TWCHAR* a_wszString );
	static void         RemapPresenterGlyphs( TXUIElement* a_pParent, TINT a_iUnused );

	// The platform whose glyphs are shown: TInputDevice::GetLastGamepadType, Platform_* = GAMEPADTYPE
	static TINT GetPlatform();

	static void SetGameHooks( const InputBinding* a_pKeyboardBindings, const InputBinding* a_pXbox360Bindings, const InputBinding* a_pPS4Bindings )
	{
		ms_pKeyboardBindings    = a_pKeyboardBindings;
		ms_pXbox360Bindings     = a_pXbox360Bindings;
		ms_pPS4Bindings         = a_pPS4Bindings;
	}

public:
	// The glyph of each binding: the input glyph, then the glyph of each platform
	static TWCHAR ms_aPlatformGlyphs[ NUM_PLATFORM_GLYPHS ][ 1 + Platform_NUMOF ];

	static const InputBinding* ms_pKeyboardBindings;
	static const InputBinding* ms_pXbox360Bindings;
	static const InputBinding* ms_pPS4Bindings;

protected:
	TBOOL               m_bIgnorePresses;                // 0x94
	TUINT8              m_uiSceneFlags;                  // 0x95
	TXUITimelineControl m_oTimelineControl;              // 0x98
	TXUIControl*        m_apFocus[ MAXCONTROLLERS ];     // 0xE8
	TXUIControl*        m_apLastFocus[ MAXCONTROLLERS ]; // 0xF8 (not used by TXUIScene)
	TXUIControl*        m_pDefaultFocus;                 // 0x108
	TXUIBackButton*     m_pBackButton;                   // 0x10C first TXUIBackButton child
	const TWCHAR*       m_wszClassOverride;              // 0x110
	TXUIVisual*         m_pTransTo;                      // 0x114
	TXUIVisual*         m_pTransFrom;                    // 0x118
	TXUIVisual*         m_pTransBackTo;                  // 0x11C
	TXUIVisual*         m_pTransBackFrom;                // 0x120
	TUINT32             m_uiInterruptTransitions;        // 0x124
	TXUIScene*          m_pPrevScene;                    // 0x128
	TINT                m_iTransitionIndex;              // 0x12C the transitions play the named frames "TransTo<index>"... if not 0
};

TOSHI_NAMESPACE_END
