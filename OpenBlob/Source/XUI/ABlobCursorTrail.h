#pragma once
#include <XUI/TXUIScene.h>
#include <Math/TVector2.h>

namespace FMOD
{
class Event;
}

namespace Toshi
{
class T2GUIMaterial;
}

//-----------------------------------------------------------------------------
// Purpose: the paint trail the menu cursors leave behind (the
// "CursorTrail" controls of frontend.xur and the loading screens; up to MAX_CURSOR_ARRAY of them). The cursor
// positions are stored in a ring of MAX_POINTS points that fade out; every frame the points are drawn as a
// Hermite spline strip with the texture cursortrail.tga and a round cap at the head. Now and then the trail drips
// ("CursorTrailDrip" scenes) or splats ("CursorTrailSplat" scenes), and moving it plays the looping paint sound
// of the loading sounds. Keyboard: X changes the colour, C erases.
//-----------------------------------------------------------------------------
class ABlobCursorTrail : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobCursorTrail, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT MAX_CURSOR_ARRAY = 8;
	static TCONSTEXPR_VAR TINT MAX_POINTS       = 256;
	static TCONSTEXPR_VAR TINT MAX_VERTICES     = 512;
	static TCONSTEXPR_VAR TINT NUM_DRIPS        = 8;
	static TCONSTEXPR_VAR TINT NUM_SPLATS       = 8;
	static TCONSTEXPR_VAR TINT NUM_SPEEDS       = 12;
	static TCONSTEXPR_VAR TINT NUM_COLOURS      = 7;

	// 0x18: a point of the trail
	struct Point
	{
		Toshi::TVector2 vPos;     // 0x00
		TFLOAT          fDistSq;  // 0x08 squared distance to the previous point
		TFLOAT          fLife;    // 0x0C fades from 10 to 0
		TFLOAT          fUnk10;   // 0x10
		TUINT32         uiColour; // 0x14
	};

	// 0x08: a drip or a splat scene and the point it's attached to (-1: unused)
	struct Effect
	{
		Toshi::TXUIScene* pScene; // 0x00
		TINT              iPoint; // 0x04
	};

public:
	ABlobCursorTrail();
	~ABlobCursorTrail();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void Render() override;                    // +0x14

	// Hiding the trail resets it
	virtual void SetVisible( TBOOL a_bVisible ) override; // +0x48

	// Clears the trail and the effects
	void Reset();

	// Creates the drips and the splats of the level the trail is in (frontend, loading screen or level)
	void CreateEffects();

	// Stops the paint sound (a_bPlayEnd plays its end sound)
	void StopSound( TBOOL a_bPlayEnd );

	// Makes the points fade out faster
	void ClampFades();

	void SetIndex( TINT a_iIndex ) { m_iIndex = TINT8( a_iIndex ); }

	// All the trails
	static void ClampAllFades();
	static void ResetAll();

	// Hermite spline of a_pPoints (point 1, point 2, tangent 1, tangent 2) at a_fT
	static Toshi::TVector2* EvaluateHermite( Toshi::TVector2* a_pOut, const Toshi::TVector2* a_pPoints, TFLOAT a_fT );

	// Bezier curve of degree a_iDegree of a_pPoints at a_fT
	static Toshi::TVector2* EvaluateBezier( Toshi::TVector2* a_pOut, const Toshi::TVector2* a_pPoints, TINT a_iDegree, TFLOAT a_fT );

private:
	// Clears the drip at the head of the ring
	void StopDripAtHead();

private:
	static ABlobCursorTrail* sm_apCursorTrails[ MAX_CURSOR_ARRAY ];

private:
	Toshi::TVector2       m_vPrevPos;               // 0x130 the cursor position of the last frame
	Toshi::TVector2       m_vLastPoint;             // 0x138 the position of the last point
	Point                 m_aPoints[ MAX_POINTS ];  // 0x140
	TINT                  m_iHead;                  // 0x1940
	TINT                  m_iNumPoints;             // 0x1944
	Toshi::TVector2*      m_pVertices;              // 0x1948
	Toshi::TVector2*      m_pUVs;                   // 0x194C
	TUINT32*              m_pColours;               // 0x1950
	TINT                  m_iMaxVertices;           // 0x1954
	Effect                m_aDrips[ NUM_DRIPS ];    // 0x1958
	TINT                  m_iNextDrip;              // 0x1998
	Effect                m_aSplats[ NUM_SPLATS ];  // 0x199C
	TINT                  m_iNextSplat;             // 0x19DC
	TBOOL                 m_bReset;                 // 0x19E0 the next Tick starts the trail at the cursor
	TBOOL                 m_bErase;                 // 0x19E1 C is down: the trail erases
	TINT8                 m_iIndex;                 // 0x19E2 the trail of the frontend (0 to 3)
	TINT                  m_iColour;                // 0x19E4 index into the colours
	Toshi::T2GUIMaterial* m_pMaterial;              // 0x19E8 cursortrail.tga
	TFLOAT                m_fSplatTimer;            // 0x19EC
	TFLOAT                m_fFadeSpeed;             // 0x19F0
	TINT                  m_iNumVisible;            // 0x19F4 the points that are still alive
	TBOOL                 m_bSoundStarted;          // 0x19F8
	TFLOAT                m_fSoundParam;            // 0x19FC
	TFLOAT                m_afSpeeds[ NUM_SPEEDS ]; // 0x1A00 the cursor speeds of the last frames
	TINT                  m_iSpeedHead;             // 0x1A30
	Toshi::TVector2       m_vSoundPos;              // 0x1A34
	Toshi::TVector2       m_vSoundPrevPos;          // 0x1A3C
	FMOD::Event*          m_pEvent;                 // 0x1A44 the paint sound
};
