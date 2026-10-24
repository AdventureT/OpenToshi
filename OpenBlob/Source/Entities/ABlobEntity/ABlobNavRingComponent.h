#pragma once

class ABuildingInstance;
#include <Math/TVector4.h>

class ABlobEntity;

namespace Toshi
{
class TXUIScene;
}

//-----------------------------------------------------------------------------
// Purpose: Blob's navigation ring (ABlobNavRingComponent.cpp, no vftable, ABlobEntity+0x3E4): while it is shown, the
// paint trail draws its spinning arrow (ABlobRibbonTrailComponent FLAGS_ARROW) and five "NavRingIcon" scenes of
// ingame.xur point at the current target, the nearest water and the challenges. While it is shown the trail keeps no
// segments (ABlobEntity::PostPhysicsUpdate). It fades in (STATE_SHOWING) and out (STATE_HIDING) in s_fFadeTime.
//-----------------------------------------------------------------------------
class ABlobNavRingComponent
{
public:
	enum STATE : TINT
	{
		STATE_SHOWING = 0,
		STATE_SHOWN   = 1,
		STATE_HIDING  = 2,
		STATE_HIDDEN  = 3,
	};

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_ICON0_DISPLAY  = BITFLAG( 0 ), // m_apIconScenes[ 2 ] (m_apIconTargets[ 0 ]) plays "Display"
		FLAGS_ICON1_DISPLAY  = BITFLAG( 1 ), // m_apIconScenes[ 3 ]
		FLAGS_ICON2_DISPLAY  = BITFLAG( 2 ), // m_apIconScenes[ 4 ]
		FLAGS_TARGET_DISPLAY = BITFLAG( 3 ), // m_apIconScenes[ 0 ]
		FLAGS_WATER_DISPLAY  = BITFLAG( 4 ), // m_apIconScenes[ 1 ]
	};

	static TCONSTEXPR_VAR TINT NUM_ICON_SCENES = 5;

public:
	ABlobNavRingComponent( ABlobEntity* a_pBlob );

	// $deBlob: FUNCTION 007be200
	// $deBlob: FUNCTION 00751390
	~ABlobNavRingComponent() {}

	// Updates the arrow of the paint trail and the icons
	void Tick( TFLOAT a_fDeltaTime );

	// Fades the ring in (from STATE_HIDING or STATE_HIDDEN)
	void Show();

	// Fades the ring out (from STATE_SHOWING or STATE_SHOWN)
	void Hide();

	STATE GetState() const { return m_eState; }

private:
	void ResetTarget();
	void ResetIconTargets();
	void ResetWaterTarget();
	void ResetBuildingTarget();

	void UpdateTarget( TFLOAT a_fDeltaTime );
	// The target of the level mode 2: the nearest unpainted building, a few buildings per update
	void UpdateBuildingTarget( TFLOAT a_fDeltaTime );
	void UpdateWaterTarget( TFLOAT a_fDeltaTime );
	void UpdateIcons( TFLOAT a_fDeltaTime );

	// Places a_pScene on the ring in the direction of a_rcTarget, scaled by a_fScale and the camera
	void PlaceIcon( Toshi::TXUIScene* a_pScene, const Toshi::TVector4& a_rcTarget, TFLOAT a_fScale );
	// Places a_pScene on the entity at *a_ppTarget (cleared once it's gone, FLAG 0x400000) and plays "Display"/"Normal"
	// as a_uiFlag changes
	void UpdateIconScene( Toshi::TXUIScene* a_pScene, FLAGS a_uiFlag, void** a_ppTarget );
	void SetTargetIconDisplayed( TBOOL a_bDisplayed );
	void SetWaterIconDisplayed( TBOOL a_bDisplayed );
	static Toshi::TXUIScene* __stdcall CreatePaintBotIconScene( TUINT32 a_uiColour );

private:
	ABlobEntity*       m_pBlob;                           // 0x00
	STATE              m_eState;                          // 0x04
	FLAGS              m_uiFlags;                         // 0x08
	TFLOAT             m_fAlpha;                          // 0x0C the scale of the arrow, 1e-6 when hidden
	TFLOAT             m_fTargetTimer;                    // 0x10
	void*              m_pCurrentChallenge;               // 0x14
	TUINT32            m_auiChallengeTarget[ 4 ];         // 0x18
	void*              m_pTargetEntity;                   // 0x28
	void*              m_pNearestChallenge;               // 0x2C
	void*              m_pChallengeEntity;                // 0x30
	TFLOAT             m_fIconTimer;                      // 0x34 not set by the constructor
	void*              m_apIconTargets[ 3 ];              // 0x38
	void*              m_pWaterTarget;                    // 0x44 the nearest water hydrant (UpdateIcons)
	void*              m_pWaterMarker;                    // 0x48 the nearest water marker (ANavMarkers::Marker)
	TFLOAT             m_fWaterTimer;                     // 0x4C not set by the constructor
	TINT               m_iBuildingIndex;                  // 0x50 the next building UpdateBuildingTarget looks at
	ABuildingInstance* m_pTargetBuilding;                 // 0x54 the nearest unpainted building (UpdateBuildingTarget)
	Toshi::TXUIScene*  m_apIconScenes[ NUM_ICON_SCENES ]; // 0x58
	TINT               m_iTargetIcon;                     // 0x6C index of s_awszIcons
	TINT               m_iShownTargetIcon;                // 0x70
};                                                        // 0x74
