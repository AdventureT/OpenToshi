#pragma once
#include "ACamera.h"
#include "ANedCameraFunction.h"
#include "ASSCameraFunction.h"

class ABlobEntity;

TOSHI_NAMESPACE_START
class TPhysicsPhantomContainer;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the camera following a player's blob, created by APlayerManager for each player. Update picks a position
// handler (POS_HANDLE_*: the normal follow, Ned's fixed cameras, splines, water...), moves the camera position towards
// the handler's ideal one and turns it to the blob.
// NOTE: only the normal follow (POS_HANDLE_FOLLOW) is ported
//-----------------------------------------------------------------------------
class ABlobFollowCamera : public ACamera
{
public:
	TDECLARE_CLASS( ABlobFollowCamera, ACamera );

	friend class APlayerManager; // the camera states

public:
	static TCONSTEXPR_VAR TINT MAX_ALPHA_INSTANCES = 20;

	// The position handlers (iPosType)
	typedef TINT POS_HANDLE;
	enum POS_HANDLE_ : POS_HANDLE
	{
		POS_HANDLE_NONE,
		POS_HANDLE_1,
		POS_HANDLE_2,
		POS_HANDLE_3,
		POS_HANDLE_NEDFIXED,
		POS_HANDLE_FOLLOW, // the normal follow
		POS_HANDLE_SPLINE,
		POS_HANDLE_7,
		POS_HANDLE_8,
		POS_HANDLE_9,
		POS_HANDLE_10,
	};

	// The speeds of a position handler (4 floats each)
	struct PosSpeeds
	{
		TFLOAT m_fAccel;      // 0x0 scaled by the camera speed option
		TFLOAT m_fMaxSpeed;   // 0x4 scaled by the camera speed option
		TFLOAT m_fDampenDist; // 0x8 divided by the camera speed option
		TFLOAT m_fLerpSpeed;  // 0xC scaled by the camera speed option when it's > 1
	};

	enum PosSpeedSet
	{
		PS_FOLLOW,
		PS_3,
		PS_3_FLAG80,
		PS_NEDFIXED,
		PS_NEDFIXED_10_20,
		PS_NEDFIXED_2,
		PS_SPLINE,
		PS_1,
		PS_2,
		PS_7,
		PS_UNUSED,
		PS_8,
		PS_9,
		PS_10,
		PS_NUMOF
	};

public:
	ABlobFollowCamera();
	~ABlobFollowCamera();

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x0C
	virtual void OnSuspend() override;                   // +0x18
	virtual void Invalidate() override;                  // +0x1C
	virtual void OnDeactivate() override;                // +0x20
	virtual void OnDestroy() override;                   // +0x24

	void SetTarget( ABlobEntity* a_pBlob );

	// The position handler 2 has brought the camera to its position (within sqrt( 3))
	// $deBlob: FUNCTION 0058c360
	TBOOL IsAtIdealPos() const { return m_iPosType == POS_HANDLE_2 && Toshi::TVector4::DistanceSq( m_vIdealPos, GetMatrix().GetTranslation() ) < 3.0f; }

	// The camera is on a Ned fixed camera with the flag 0x800
	// $deBlob: FUNCTION 0058c3a0
	ANedCameraFunction::FixedCam* GetNedFixedCam() const { return m_pNedFixedCam; }

	TBOOL IsOnFlaggedNedFixedCam() const { return m_iPosType == POS_HANDLE_NEDFIXED && m_pNedFixedCam != TNULL && ( m_pNedFixedCam->m_iFLAGS & 0x800 ); }

	// Snaps the camera to a start position for the blob
	void Reset( TBOOL a_bLevelStart, TBOOL a_bRespawn );

	// Sets m_bUnk1C1 (only with a blob; then also m_uiControls = 4)
	void SetUnk1C1( TBOOL a_bUnk1C1 );
	void SetFromCamera( ACamera* a_pCamera );
	void SetOverrideCamera( ACamera* a_pCamera );

	// Blends the rotation in after a camera change
	void StartRotationBlend();

	// Sets the tuning values (called by ACameraManager's constructor)
	static void InitialiseSettings();

private:
	// The position of the normal follow
	static void UpdateFollowPosition( TFLOAT a_fDeltaTime, Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvCamPos, const Toshi::TVector4& a_rvSmoothPos, const Toshi::TVector4& a_rvFacing, const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvBlobPos, TFLOAT a_fSpeed, TFLOAT a_fMaxSpeed, TFLOAT a_fRadius, TFLOAT a_fMinDist, TFLOAT a_fIdealDist, TFLOAT a_fMaxDist, TBOOL a_bInLiquid, TFLOAT* a_pfTimer1, TFLOAT* a_pfTimer2, void* a_pPhantom1, void* a_pPhantom2 );

	// Destroys the two phantoms and forgets the blob
	void DestroyPhantoms();
	// The speeds of the Ned fixed camera for the position handler flags a_iFlags (2: PS_NEDFIXED_2, 0x10/0x20:
	// PS_NEDFIXED_10_20, else PS_NEDFIXED)
	static void GetNedFixedSpeeds( TFLOAT* a_pfAccel, TFLOAT* a_pfMaxSpeed, TFLOAT* a_pfDampenDist, TFLOAT* a_pfLerpSpeed, TINT a_iFlags );
	// Moves a_fValue towards a_fTarget by the difference times the speed (a_fSpeedDown below, a_fSpeedUp above) and
	// a_fDeltaTime; TRUE (and a_fTarget) once it gets there
	static TBOOL MoveTowards( TFLOAT* a_pfOut, TFLOAT a_fValue, TFLOAT a_fTarget, TFLOAT a_fSpeedDown, TFLOAT a_fSpeedUp, TFLOAT a_fDeltaTime );

	// The region categories of a_eCategory (0: 0x1E7E, 1: 1)
	static TUINT32 GetRegionsForCategory( TINT a_eCategory );

	// The fixed (or, a_bSpline, spline) camera for the blob's situation (help, cannon, z-jump...) in the regions at
	// a_rvPos; the categories searched in a_puiSearched, the one found in a_puiFlag (0x10000: none)
	static void SearchCameras( TINT a_eCategory, ABlobEntity* a_pBlob, ANedCameraFunction* a_pNed, const Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvViewPos, TUINT32* a_puiSearched, TUINT32* a_puiFlag, ANedCameraFunction::FixedCam** a_ppFixedCam, ANedCameraFunction::SplineCam** a_ppSplineCam, TBOOL a_bSpline );

	// Switches to the fixed camera found (after its delay) and keeps it for its hold time: its position, orientation and
	// FOV; TFALSE if there's none
	static TBOOL UpdateNedFixedCam( TFLOAT a_fDeltaTime, TINT a_eCategory, ABlobEntity* a_pBlob, ANedCameraFunction* a_pNed, const Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvViewPos, Toshi::TVector4& a_rvOutPos, Toshi::TQuaternion& a_rqOutOrient, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed, ANedCameraFunction::FixedCam** a_ppLast, ANedCameraFunction::FixedCam** a_ppCurrent, ANedCameraFunction::FixedCam** a_ppPending, TFLOAT* a_pfTimer, TUINT32* a_puiFlags );

	// The same for the spline cameras: the position on the scripted sequence camera
	static TBOOL UpdateSplineCam( TFLOAT a_fDeltaTime, TINT a_eCategory, ABlobEntity* a_pBlob, ANedCameraFunction* a_pNed, ASSCameraFunction* a_pSSCams, const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvViewPos, Toshi::TVector4& a_rvOutPos, Toshi::TVector4* a_pvFollowPos, Toshi::TQuaternion* a_pqOutOrient, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed, ANedCameraFunction::SplineCam** a_ppLast, ANedCameraFunction::SplineCam** a_ppCurrent, ANedCameraFunction::SplineCam** a_ppPending, TFLOAT* a_pfTimer, TUINT32* a_puiFlags );

	// A challenge with its camera (AChallengeManager): the challenge type's distance, pitch and FOV around the challenge
	// entity
	static TBOOL UpdateChallengeCam( TFLOAT a_fDeltaTime, ABlobEntity* a_pBlob, Toshi::TVector4* a_pvTarget, Toshi::TVector4& a_rvOutPos, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed );

	// An animation with flag 2 plays on the blob's character model: the camera looks at its body bone from above
	// behind
	static TBOOL UpdateAnimationCam( ABlobEntity* a_pBlob, Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvTarget, Toshi::TQuaternion* a_pqOrient, TFLOAT* a_pfFOV, TFLOAT* a_pfFOVSpeed, TBOOL* a_pbSnap );

	// The camera controls (m_uiControls from ReadInput): turning around the target (1/2), behind the blob (0x80); the
	// position stays for a while (a_pbActive, a_pvLastTarget)
	static TBOOL UpdateManualCam( TFLOAT a_fDeltaTime, Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvCamPos, const Toshi::TVector4& a_rvSmoothPos, const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvFacing, Toshi::TVector4* a_pvLastTarget, TBOOL* a_pbActive, TUINT32 a_uiInput, TUINT32* a_puiLastInput, TFLOAT a_fMinDist, TFLOAT a_fCollisionRadius, TFLOAT a_fMaxDist, TFLOAT* a_pfTurnSpeed, TFLOAT* a_pfBlendTime );
#ifdef DEBLOB_COMMUNITY_PATCH
	static void  PitchManualCam( Toshi::TVector4& a_rvDir, TFLOAT a_fAngle, TFLOAT a_fDeltaTime );
#endif

	// Debug (ToggleUnderCam): a camera far below the target
	static TBOOL UpdateUnderCam( ABlobEntity* a_pBlob, Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvCamPos, const Toshi::TVector4& a_rvSmoothPos, const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvFacing, TFLOAT a_fRadius, TFLOAT a_fCollisionRadius );

	// Debug menu callback switching UpdateUnderCam
	static TBOOL ToggleUnderCam( void* a_pUnused );

	// The blob's m_bInBlobVision is set: the camera right at the target
	static TBOOL UpdateCloseCam( ABlobEntity* a_pBlob, const Toshi::TVector4& a_rvTarget, Toshi::TVector4& a_rvPos );

	// The blob is busy (player data m_uiLives > 0) or (flag 0x10000000) in a 0x800 region: a camera close behind it
	// (unless a_iPosType is POS_HANDLE_2 already)
	static TBOOL UpdateBusyCam( ABlobEntity* a_pBlob, const Toshi::TVector4& a_rvTarget, Toshi::TVector4& a_rvPos, POS_HANDLE a_iPosType );

	// Reads the camera controls of the blob's player into m_afFreeLookInput/m_uiControls
	void ReadInput( TFLOAT a_fDeltaTime );

	// The FOV from the blob's speed and direction
	static void UpdateFollowFOV( const Toshi::TVector4& a_rvCamPos, const Toshi::TVector4& a_rvBlobPos, Toshi::TVector4 a_vVelocity, TFLOAT a_fMaxPlayerSpeed, TFLOAT* a_pfFOV, TBOOL a_bNarrow );

	// The region distances (a_bReset: take them at once)
	static void UpdateRegionDistances( TFLOAT a_fDeltaTime, ANedCameraFunction* a_pRegions, const Toshi::TVector4& a_rvTarget, TFLOAT a_fRadius, TFLOAT* a_pfMinDist, TFLOAT* a_pfIdealDist, TFLOAT* a_pfMaxDist, TFLOAT* a_pfTimer, TUINT32* a_puiRegionData, void** a_ppRegion, TBOOL a_bReset );

	// Moves the smoothed position towards the ideal one
	// Casts a ray of a_rfDist from a_rvFrom along a_rvDir; on a hit returns the normal and shortens a_rfDist to the hit.
	static TBOOL CastRay( const Toshi::TVector4& a_rvFrom, const Toshi::TVector4& a_rvDir, Toshi::TVector4& a_rvOutNormal, TFLOAT& a_rfDist, TUINT32 a_uiCollisionLayer, Toshi::TPhysicsPhantomContainer* a_pPhantom );
	// Sweeps a sphere of a_fRadius like CastRay.
	static TBOOL SphereCast( const Toshi::TVector4& a_rvFrom, const Toshi::TVector4& a_rvDir, Toshi::TVector4& a_rvOutNormal, TFLOAT& a_rfDist, TFLOAT a_fRadius, TUINT32 a_uiCollisionLayer );
	// Something above the blob (or a_pfTimer running): moves the camera low behind it, under the ceiling
	static TBOOL AvoidCeiling( const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvBlobPos, Toshi::TVector4& a_rvPos, TFLOAT* a_pfTimer, TFLOAT a_fMinDist, TFLOAT a_fIdealDist, TFLOAT a_fMaxDist, TFLOAT a_fRadius, TFLOAT a_fCollisionRadius );
	// Too close or blocked: searches 16 directions around the target for an open one, behind the blob first
	static void FindOpenDirection( const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvFacing, Toshi::TVector4& a_rvPos, TFLOAT a_fMinDist, TFLOAT a_fIdealDist, TFLOAT a_fMaxDist, TFLOAT a_fCollisionRadius );
	// Turns away from walls on one side of the camera
	static void AvoidSideWalls( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rvTarget, Toshi::TVector4& a_rvPos, TFLOAT a_fMaxDist, TFLOAT a_fIdealDist, TFLOAT a_fCollisionRadius, TBOOL a_bSnap, void* a_pPhantom1, void* a_pPhantom2 );
	// The smoothed position was blocked on its way: slides it along the line from the ideal position to the target
	static void   SlideAroundObstacle( Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvFrom, const Toshi::TVector4& a_rvIdealPos, const Toshi::TVector4& a_rvTarget, TFLOAT a_fCollisionRadius );
	static void   SmoothPosition( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rvCamPos, Toshi::TVector4& a_rvSmoothPos, const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvIdealPos, Toshi::TVector4& a_rvVelocity, TFLOAT a_fAccel, TFLOAT a_fMaxSpeed, TFLOAT a_fDampenDist, TFLOAT a_fRadius, TBOOL a_bCheckCollision );
	static TFLOAT MoveTowards( const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvFrom, const Toshi::TVector4& a_rvTo, Toshi::TVector4& a_rvOut, TFLOAT a_fDist );
	static TFLOAT GetTravelDistance( TFLOAT a_fSecs, TFLOAT a_fAccel, TFLOAT a_fMaxSpeed, TFLOAT a_fDampeningDist, TFLOAT a_fDist, TFLOAT* a_pfCurrentSpeed );
	static TFLOAT GetDampenedTravelDistance( TFLOAT a_fSecs, TFLOAT a_fAccel, TFLOAT a_fMaxSpeed, TFLOAT a_fDampeningDist, TFLOAT a_fDist, TFLOAT* a_pfCurrentSpeed );
	static void   LerpPosition( TFLOAT a_fDeltaTime, Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvTo, TFLOAT a_fSpeed );

	// The blended fixed camera orientation
	// NOTE: retail also takes the delta time, the rotation speed and a vector that it never reads
	static TBOOL UpdateBlendedFixedCam( const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvCamPos, const Toshi::TVector4& a_rvIdealPos, const Toshi::TMatrix44& a_rmLast, const Toshi::TQuaternion& a_rqOrient, Toshi::TMatrix44& a_rmOut, TBOOL a_bBlend );
	// Slerps the orientation a_rmOut towards the target by a_fSpeed * a_fDeltaTime added to the blend a_pfBlend (0..1), its
	// position is kept
	static void UpdateTurnTowards( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, const Toshi::TVector4& a_rvTarget, Toshi::TMatrix44& a_rmOut );
	static void UpdateBlendedOrient( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, const Toshi::TQuaternion& a_rqOrient, Toshi::TMatrix44& a_rmOut );
	static void UpdateOffsetLookAt( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, TFLOAT a_fAngle, const Toshi::TVector4& a_rvFrom, const Toshi::TVector4& a_rvTo, const Toshi::TVector4& a_rvUnused, Toshi::TMatrix44& a_rmOut );
	static void UpdateFreeLook( TFLOAT a_fDeltaTime, Toshi::TVector4& a_rvDir, Toshi::TVector4& a_rvUp, const Toshi::TVector4& a_rvRight, const TFLOAT* a_pfInput, TBOOL a_bActive );
	static void UpdateBlendedLookAt( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed, TFLOAT* a_pfBlend, const Toshi::TVector4& a_rvTarget, Toshi::TMatrix44& a_rmOut );
	static void UpdateLookDirection( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rvCamPos, const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvFacing, Toshi::TVector4& a_rvDir, Toshi::TVector4& a_rvUp, const Toshi::TVector4& a_rvRight, TFLOAT* a_pfRotSpeed, TFLOAT* a_pfPitch, TFLOAT* a_pfYaw, TFLOAT a_fFOV, TFLOAT a_fVFOV, TBOOL a_bNoOffset, TBOOL a_bSnap, TFLOAT a_fMinBlend );

public:
	// The camera speed option (AOptions::Apply), clamped to [0.5, 1.0]
	// $deBlob: FUNCTION 0058c250
	static TFLOAT GetCamSpeedOption() { return ms_fCamSpeedOption; }

	// -1: the inverted camera option
	// $deBlob: FUNCTION 0058c350
	static TFLOAT GetInvertY() { return ms_fInvertY; }

	// The range of the camera speed option
	static TFLOAT GetMINCamSpeedOption();
	static TFLOAT GetMAXCamSpeedOption();

	// $deBlob: FUNCTION 0058c260
	static void SetCamSpeedOption( TFLOAT a_fSpeed )
	{
		a_fSpeed = Toshi::TMath::Min( Toshi::TMath::Max( a_fSpeed, 0.5f ), 1.0f );
		TASSERT( a_fSpeed >= 0.5f && a_fSpeed <= 1.0f );
		ms_fCamSpeedOption = a_fSpeed;
	}

	// The inverted camera option (AOptions::Apply)
	// $deBlob: FUNCTION 0058c320
	static void SetInvertCamera( TBOOL a_bInverted ) { ms_fInvertY = a_bInverted ? -1.0f : 1.0f; }

	// The camera is inverted when the invert factor is not positive
	// $deBlob: FUNCTION 0058c300
	static TBOOL IsInvertCamera() { return ms_fInvertY <= 0.0f; }

public:
	// Tuning values (InitialiseSettings)
	static TFLOAT    ms_fCamSpeedOption;
	static TFLOAT    ms_fInvertY;
	static TFLOAT    ms_fCollisionRadius;
	static TFLOAT    ms_fDefaultMaxDist;
	static TFLOAT    ms_fDefaultIdealDist;    // sfCamIdealDist
	static TFLOAT    ms_fAcceptableIdealDist; // sfCamAccepatableIdealDist
	static TFLOAT    ms_fDefaultMinDist;
	static TFLOAT    ms_fRotSpeed;
	static TFLOAT    ms_fUnkBC2C;
	static TFLOAT    ms_fUnkBC30;
	static TFLOAT    ms_fPitchFactor;
	static TFLOAT    ms_fLookAngleFactor;
	static TFLOAT    ms_afLookSpeeds[ 8 ];
	static TFLOAT    ms_fAnimCamDist;
	static TFLOAT    ms_fAnimCamHeight;
	static TFLOAT    ms_fAnimCamLookDown;
	static TFLOAT    ms_fUnkBC68;
	static TFLOAT    ms_fRotBlendDown;
	static TFLOAT    ms_fRotBlendUp;
	static TFLOAT    ms_fRotBlendTime;
	static TFLOAT    ms_fYawFactor;
	static TFLOAT    ms_fOrbitSpeed;
	static TFLOAT    ms_fFOVMin;
	static TFLOAT    ms_fMinPitch;
	static TFLOAT    ms_fMaxPitch;
	static TFLOAT    ms_fFOVBlendSpeed;
	static TFLOAT    ms_fUnkF18;
	static TFLOAT    ms_fOrbitPitch;
	static TFLOAT    ms_fFOVMax;
	static TFLOAT    ms_fLiquidPitch;
	static TBOOL     ms_bUnderCam;
	static PosSpeeds ms_aPosSpeeds[ PS_NUMOF ];

	// The instances between the camera and Blob that fade out (MAX_ALPHA_INSTANCES = MAX_ALPHA_INSTANCES of them): the array at 0x254
	// holds AModelInstance2 pointers (declared as floats), 0x2A4 the current alpha of each, 0x2F4 the alpha to go back to
	// (ARenderer::MainScene)
	// Forgets a_pInstance in the fading instances (it's unloaded)
	// $deBlob: FUNCTION 0058c0b0
	void ForgetAlphaInstance( class AModelInstance2* a_pInstance )
	{
		AModelInstance2** apInstances = TREINTERPRETCAST( class AModelInstance2**, m_afAlphaInstances );

		TINT              i          = MAX_ALPHA_INSTANCES;
		AModelInstance2** ppInstance = apInstances + MAX_ALPHA_INSTANCES;

		while ( i != 0 )
		{
			ppInstance--;
			i--;

			if ( *ppInstance == a_pInstance )
				*ppInstance = TNULL;
		}
	}

	// $deBlob: FUNCTION 0058c0e0
	class AModelInstance2** GetAlphaInstances( TINT& a_rCount )
	{
		a_rCount = MAX_ALPHA_INSTANCES;
		return TREINTERPRETCAST( class AModelInstance2**, m_afAlphaInstances );
	}

	// $deBlob: FUNCTION 0058c100
	void SetAlphaTarget( TINT a_iIndex, TFLOAT a_fAlpha )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < MAX_ALPHA_INSTANCES );
		m_afAlphaTarget[ a_iIndex ] = a_fAlpha;
	}

	// $deBlob: FUNCTION 0058c170
	TFLOAT GetAlphaTarget( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < MAX_ALPHA_INSTANCES );
		return m_afAlphaTarget[ a_iIndex ];
	}

	// $deBlob: FUNCTION 0064a260
	void SetBlobVision( TBOOL a_bBlobVision ) { m_bBlobVision = a_bBlobVision; } // ABlobVisionHud

	// $deBlob: FUNCTION 0058c1d0
	TFLOAT GetAlpha( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < MAX_ALPHA_INSTANCES );
		return m_afAlpha[ a_iIndex ];
	}

private:
	ANedCameraFunction::SplineCam* m_pLastSpline;         // 0x104
	ANedCameraFunction::SplineCam* m_pCurrSpline;         // 0x108
	ANedCameraFunction::SplineCam* m_pPendingSpline;      // 0x10C
	TUINT32                        m_uiSplineFlags;       // 0x110 0x10000: none, else the category of m_pCurrSpline
	TFLOAT                         m_fSplineTimer;        // 0x114
	Toshi::TVector4                m_vUnk118;             // 0x118 the blob's position when targeted
	ANedCameraFunction::FixedCam*  m_pNedFixedCam;        // 0x128
	ANedCameraFunction::FixedCam*  m_pPendingFixedCam;    // 0x12C
	ANedCameraFunction::FixedCam*  m_pLastFixedCam;       // 0x130
	TUINT32                        m_uiFlags;             // 0x134 1 (0x10000 when targeted)
	void*                          m_pRegion;             // 0x138 the current camera region
	TFLOAT                         m_fMaxDist;            // 0x13C
	TFLOAT                         m_fIdealDist;          // 0x140
	TFLOAT                         m_fMinDist;            // 0x144
	TFLOAT                         m_fRegionTimer;        // 0x148
	TUINT32                        m_uiRegionData;        // 0x14C
	ABlobEntity*                   m_pBlob;               // 0x150
	Toshi::TVector4                m_vVelocity;           // 0x154 of the smoothed position
	Toshi::TVector4                m_vIdealPos;           // 0x164 the position handler's position
	Toshi::TVector4                m_vSmoothPos;          // 0x174 moved towards m_vIdealPos
	Toshi::TVector4                m_vUnk184;             // 0x184
	Toshi::TVector4                m_vLastPos;            // 0x194 the camera position when the handler changed
	Toshi::TVector4                m_vUnk1A4;             // 0x1A4
	TBOOL                          m_bUnk1B4;             // 0x1B4
	TFLOAT                         m_afFreeLookInput[ 2 ]; // 0x1B8
	TBOOL                          m_bFreeLook;            // 0x1C0
	TBOOL                          m_bUnk1C1;              // 0x1C1 cleared by Invalidate
	TFLOAT                         m_fFixedCamTimer;       // 0x1C4
	TFLOAT                         m_fPitch;               // 0x1C8
	TFLOAT                         m_fYaw;                 // 0x1CC
	TFLOAT                         m_fLookSpeed;           // 0x1D0
	TFLOAT                         m_fManualRotationSpeed; // 0x1D4 the manual rotation speed (-1..1)
	TUINT32                        m_uiControls;           // 0x1D8
	TUINT32                        m_uiUnk1DC;             // 0x1DC
	POS_HANDLE                     m_iPosType;             // 0x1E0 of the last update
	TUINT32                        m_uiUnk1E4;             // 0x1E4
	TFLOAT                         m_fSplineBlend;         // 0x1E8 the blend of a blended spline camera (0x2000 without 0x4000)
	TUINT32                        m_uiUnk1EC;             // 0x1EC
	TFLOAT                         m_fUnk1F0;              // 0x1F0 counts down
	TFLOAT                         m_fUnk1F4;             // 0x1F4 counts down
	TFLOAT                         m_fUnk1F8;             // 0x1F8 counts down
	TFLOAT                         m_fUnk1FC;             // 0x1FC
	TFLOAT                         m_fIdleTime;           // 0x200 0-1, grows while the blob stands still
	Toshi::TMatrix44               m_mLastOrient;         // 0x204
	TFLOAT                         m_fLastFOV;            // 0x244
	void*                          m_pPhantom1;           // 0x248
	void*                          m_pPhantom2;           // 0x24C
	TUINT32                        m_uiUnk250;            // 0x250
	TFLOAT                         m_afAlphaInstances[ MAX_ALPHA_INSTANCES ]; // 0x254 0.0f
	TFLOAT                         m_afAlpha[ MAX_ALPHA_INSTANCES ];          // 0x2A4 1.0f
	TFLOAT                         m_afAlphaTarget[ MAX_ALPHA_INSTANCES ];    // 0x2F4 1.0f
	TBOOL                          m_abUnk344[ MAX_ALPHA_INSTANCES ];         // 0x344
	TBOOL                          m_bBlobVision;                             // 0x358 TTRUE
	ACamera*                       m_pOverrideCam;                            // 0x35C a scripted camera blending out into this one
	Toshi::TMatrix44               m_mPrevMatrix;                             // 0x360 the matrix of the last update
	TBOOL                          m_bRotBlend;                               // 0x3A0
	TFLOAT                         m_fRotBlend;                               // 0x3A4 100.0f
	TFLOAT                         m_fRotBlendTarget;                         // 0x3A8 1000.0f
	TFLOAT                         m_fRotBlendTimer;                          // 0x3AC 1000.0f
	TINT                           m_iUnk3B0;             // 0x3B0 1
};

TSTATICASSERT( sizeof( ABlobFollowCamera ) == 0x3B4 );
