#pragma once
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: the physics test harness (the singleton ACollisionTest, 4 bytes: only its vftable).
// RunTimingTests times the collision queries around Blob.
// NOTE: retail never creates the singleton nor calls any of these functions (no references), so they're dead code
//-----------------------------------------------------------------------------
class ACollisionTest
    : public Toshi::TSingleton<ACollisionTest>
{
public:
	ACollisionTest();
	virtual ~ACollisionTest();

	// Times 100 ray casts and 100 sphere sweeps up from Blob's body, with and without a phantom (once, when
	// ms_bRunTimingTests is set), and prints the times
	static void RunTimingTests();

	// The surface name of the ground 20 m above Blob's body (a ray up; "default" when nothing is hit or once
	// ms_bTestSurface is cleared)
	static const TCHAR* GetSurfaceAboveBlob();

	// Times THPTimer::Update (once, when ms_bRunTimerTest is set)
	static void RunTimerTest();

	// Times TMath's Sin, Cos and SinCos (once, when ms_bRunMathTests is set)
	static void RunMathTests();

	// A physics test scene: a fixed anchor sphere and a column of 20 bodies (spheres, boxes, capsules, cylinders and
	// a list shape), two of them constrained to the anchor (a ball and socket, a hinge)
	static void CreateTestScene();
	static void DestroyTestScene();
	// Puts the bodies back below Blob when the controller's context button 3 was pressed
	static void UpdateTestScene( TFLOAT a_fDeltaTime );

public:
	static TBOOL ms_bRunTimingTests;
	static TBOOL ms_bRunTimerTest;
	static TBOOL ms_bTestSurface;
	static TBOOL ms_bRunMathTests;
};
