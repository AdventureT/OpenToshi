#pragma once
#include "AGameState.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2Vector.h>

//-----------------------------------------------------------------------------
// Purpose: the stack of game states, the fades of the viewports and the widescreen bars.
//-----------------------------------------------------------------------------
class AGameStateController : public Toshi::TSingleton<AGameStateController>
{
public:
	static TCONSTEXPR_VAR TINT MAX_NUM_STATES   = 10;
	static TCONSTEXPR_VAR TINT NUM_FADEVIEWPORT = 12; // AXYZViewportManager::VIEWPORT_Count

	enum FadeType
	{
		FadeType_None,
		FadeType_ToBlack,
		FadeType_FromBlack,
	};

	struct FadeViewport
	{
		TFLOAT m_fTime;     // 0x00
		TFLOAT m_fDuration; // 0x04
		TINT   m_eFadeType; // 0x08 FadeType
		void*  m_pFade;     // 0x0C T2GUIRectangle
		TBOOL  m_bEnabled;  // 0x10
	};

public:
	AGameStateController();
	~AGameStateController();

	// Creates the fade rectangles of the viewports and the widescreen bars, pushes the base state
	void Create();
	void Destroy();
	void UnloadGUI();

	void Update( TFLOAT a_fDeltaTime );

	AGameState* GetCurrentGameState();

	// The widescreen bars on or off, at once or over a_fDuration
	void SetWidescreen( TBOOL a_bOn, TFLOAT a_fDuration );

	Toshi::T2Vector<AGameState*, MAX_NUM_STATES>& GetStateStack() { return m_oStateStack; }

	// The state of the stack whose class is a_pClass (not a derived one), TNULL if there's none
	AGameState* FindState( Toshi::TClass* a_pClass );

	void PushState( AGameState* a_pState );
	void PopState( AGameState* a_pState );

	// Pops the current state unless it's the base state
	void PopCurrentState();

	TBOOL IsRenderingFadesFirst() const { return m_bUnk13D; }
	// $deBlob: FUNCTION 005c2520
	void SetRenderFadesFirst( TBOOL a_bValue ) { m_bUnk13D = a_bValue; }
	// $deBlob: FUNCTION 005c8a80
	void SetRenderFadesWhenPaused( TBOOL a_bValue ) { m_bRenderFadesWhenPaused = a_bValue; }

	// Pops every state but the base state
	void ResetStack();

	// The fades of the viewports (there they have no viewport)
	TBOOL  IsFadeEnabled( TINT a_iViewport );
	void   EnableFade( TBOOL a_bEnable, TINT a_iViewport );
	void   SetFade( TFLOAT a_fAlpha, TINT a_iViewport );
	TFLOAT GetFade( TINT a_iViewport );
	void   FadeToBlack( TFLOAT a_fDuration, TINT a_iViewport );
	void   FadeFromBlack( TFLOAT a_fDuration, TINT a_iViewport );
	void   SetFadeColour( TUINT32 a_uiColour, TINT a_iViewport );
	TBOOL  IsFading( TINT a_iViewport );
	TBOOL  IsFadingToBlack( TINT a_iViewport );
	TBOOL  IsFadingFromBlack( TINT a_iViewport );
	void   StopFade( TINT a_iViewport );
	void   ResetFades();

	// Renders the enabled fade rectangles (last viewport first), unless an in-game state is paused (and m_bRenderFadesWhenPaused is
	// off)
	void RenderFades();

private:
	void UpdateFade( TFLOAT a_fDeltaTime );
	void UpdateWidescreen( TFLOAT a_fDeltaTime );

private:
	Toshi::T2Vector<AGameState*, MAX_NUM_STATES> m_oStateStack;                       // 0x00
	TBOOL                                        m_bStatePushing;                     // 0x2C
	FadeViewport                                 m_aFadeViewPort[ NUM_FADEVIEWPORT ]; // 0x30
	TFLOAT                                       m_fWidescreenTime;                   // 0x120
	TFLOAT                                       m_fWidescreenDuration;               // 0x124
	TBOOL                                        m_bWidescreenOn;                     // 0x128
	void*                                        m_pWidescreenTop;                    // 0x12C T2GUIRectangle
	void*                                        m_pWidescreenBottom;                 // 0x130 T2GUIRectangle
	TFLOAT                                       m_fWidescreenTopHeight;              // 0x134
	TFLOAT                                       m_fWidescreenBottomHeight;           // 0x138
	TBOOL                                        m_bRenderFadesWhenPaused;            // 0x13C
	TBOOL                                        m_bUnk13D;                           // 0x13D ARenderer::RenderGUI renders the fades before XUI
	// NOTE: the fade alpha of each viewport while its fade rectangle isn't created (m_pFade is TNULL until Create's
	// viewport part is ported); keeps the fades (and the confirmations that wait for them) running
	TFLOAT m_afFadeAlpha[ NUM_FADEVIEWPORT ];
};
