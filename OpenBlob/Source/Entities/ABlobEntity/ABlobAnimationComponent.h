#pragma once

#include <File/TTRB.h>
#include <Math/TMatrix44.h>

class ABlobEntity;
class AModelInstance2;
class ASkinMaterialHAL;

namespace Toshi
{
class TSkeleton;
class TMaterial;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: Blob's models and animations: the character model (blob_bodya), the ball models (body and ears), the
// multiplayer shield and crown, the silhouette/z-pass skin materials and the animation IDs of the shared skeleton.
// Created by ABlobEntity::Create.
//-----------------------------------------------------------------------------
class ABlobAnimationComponent
{
	friend class APropWindCannon;

public:
	// The names are in s_aszAnimStateNames
	enum STATE : TINT
	{
		STATE_BALL                = 0,
		STATE_FADETOCHAR          = 1,
		STATE_ANIMTOCHAR          = 2,
		STATE_CHAR                = 3,
		STATE_ANIMTOBALL          = 4,
		STATE_FADETOBALL          = 5,
		STATE_FLOAT               = 6,
		STATE_LOCKED              = 7,
		STATE_HOTPLATE            = 8,
		STATE_SHOCK_RECOVER       = 9,
		STATE_PAIN                = 10,
		STATE_DEAD                = 11,
		STATE_ZJUMPFAIL           = 12,
		STATE_LMARK_IN            = 13,
		STATE_LMARK_OUT           = 14,
		STATE_LEVEL_START         = 15,
		STATE_LEVEL_EXIT          = 16,
		STATE_ENTER_CANNON        = 17,
		STATE_INSIDE_CANNON       = 18,
		STATE_EXIT_CANNON         = 19,
		STATE_BRAKE               = 20,
		STATE_RESPAWN             = 21,
		STATE_RESPAWN_POOL        = 22,
		STATE_SHIELD_ACTIVATING   = 23,
		STATE_SHIELD_DEACTIVATING = 24,
		STATE_SHIELD_ACTIVE       = 25,
		STATE_CHALLENGE_VICTORY   = 26,
		STATE_CHALLENGE_DEFEAT    = 27,
		STATE_VICTORY             = 28,
		STATE_DEFEAT              = 29,
		STATE_LOSTLASTLIFE        = 30,
		STATE_POWERUP             = 31,
		STATE_GRASSBOMB           = 32,
	};

	friend class ABlobEntity;
	friend class AStencilShadow; // the shadow copies the character model's animations

	// An animation of BlobAnimationProperties (0x10 bytes)
	struct Anim
	{
		const TCHAR* m_szName;   // 0x00
		TINT16       m_iAnim;    // 0x04 the sequence of ms_pSkeletonChar
		TINT16       m_iEyeOLay; // 0x06 the "%s_eyes" overlay sequence
		TUINT32      m_uiFlags;  // 0x08
		TFLOAT       m_fSpeed;   // 0x0C
	};

	// A group of animations of BlobAnimationProperties (only the used members are mapped)
	struct AnimSet
	{
		Anim* m_pRandomAnims00;    // 0x00 one is picked at random (GetSurfaceAnim3 without a surface)
		TINT  m_iNumRandomAnims04; // 0x04
		Anim* m_pIdleAnims;        // 0x08 one is picked at random (SetState 3)
		TINT  m_iNumIdleAnims;     // 0x0C
		Anim* m_pCivAnims;         // 0x10 one is picked at random when Blob faces a civilian (ABlobEntity::UpdateModelTransforms)
		TINT  m_iNumCivAnims;      // 0x14
		Anim* m_pRandomAnims18;    // 0x18 one is picked at random (GetSurfaceLandmarkAnim without a surface)
		TINT  m_iNumRandomAnims1C; // 0x1C
		Anim* m_pRandomAnims20;    // 0x20 one is picked at random (GetSurfaceRandomAnim without a surface)
		TINT  m_iNumRandomAnims24; // 0x24
		Anim* m_pAnims;            // 0x28 indexed by the states
	};

	// The second table of BlobAnimationProperties (only the used members are mapped)
	struct AnimTable
	{
		Anim* m_pAnims00;            // 0x00 indexed by SetState 8, 9, 0xD, 0xE, 0x1F, 0x20
		Anim* m_pAnims04;            // 0x04 GetRandomUnk04Anim (SetState 0x1E) picks one at random
		TINT  m_iNumAnims08;         // 0x08
		Anim* m_pLookAnims0C;        // 0x0C indexed by m_iLookIndex (SetState 3 without a surface, the look cycle)
		TINT  m_iNumLookAnims;       // 0x10 m_iLookIndex wraps at it
		Anim* m_pLevelStartAnims;    // 0x14 SetState 0xF picks one at random
		TINT  m_iNumLevelStartAnims; // 0x18
		Anim* m_pAnims1C;            // 0x1C SetState 0x16 picks one at random
		TINT  m_iNumAnims20;         // 0x20
		Anim* m_apSurfaceAnims[ 4 ]; // 0x24 GetSurfaceAnims
		Anim* m_pLandmarkAnims;      // 0x34 GetSurfaceLandmarkAnim indexes it by the landmark ID
		TINT  m_iNumLandmarkAnims;   // 0x38
	};
	// The third table (AnimationProperties+0x08 points to it): only its first member is mapped
	struct AnimTable2
	{
		Anim* m_pAnims; // 0x00 SetState 0x17..0x19
	};

	// The "BlobAnimationProperties" symbol of BlobProperties.trb
	struct AnimationProperties
	{
		AnimSet*    m_pCommon;     // 0x00
		AnimTable*  m_pTable;      // 0x04
		AnimTable2* m_pTable2;     // 0x08
		Anim*       m_pAnims;      // 0x0C
		TINT        m_iNumAnims;   // 0x10
		Anim*       m_pSPAnims;    // 0x14 single player only
		TINT        m_iNumSPAnims; // 0x18
		Anim*       m_pMPAnims;    // 0x1C multiplayer only
		TINT        m_iNumMPAnims; // 0x20
	};

	static TCONSTEXPR_VAR TINT MESH_COUNT = 5;

public:
	ABlobAnimationComponent( ABlobEntity* a_pOwner );
	// Destroys the character and ball models, the skin materials and clears the shared skeleton
	~ABlobAnimationComponent();

	// Reads BlobAnimationProperties of BlobProperties.trb (ABlobEntity::LoadProperties); resolves the animation IDs
	// once the skeleton is known.
	static void LoadAnimationProperties( Toshi::TTRB* a_pTRB );
	static void UnloadAnimationProperties();

	// The name of an animation state ("Unknown State" out of range)
	const TCHAR* GetStateName( TINT a_eState ) const;

	void Render();

	// Updates the current state, the ball/character visibility and the character's animation (ABlobEntity::Tick)
	void        Update( TFLOAT a_fDeltaTime );
	void        SetState( TINT a_iState );
	TINT        GetState() const { return m_iState; }
	const void* GetStateAnim() const { return m_pStateAnim; }

	// The ball landed (ABlobEntity::UpdateStateTransitions): ends the jump (12) or the state 14 animation
	void OnLanded();

	// Restarts the m_fUnk28 timer (1 s; the blob got paint from a LeechBot)
	// $deBlob: FUNCTION 00755a20
	void StartUnk28Timer() { m_fUnk28 = 1.0f; }
	// Restarts the m_fCelebrateTimer timer (0.5 s; a Z-jump destroyed an entity)
	// $deBlob: FUNCTION 00755a10
	void StartCelebrateTimer() { m_fCelebrateTimer = 0.5f; }
	// Back to state 0 and the default timers/scales, hides the shield (ABlobEntity::Reset)
	void Reset();

	// Flashes the ball's skin materials with the paint colour for 0.3 s (tev mode 0x1a)
	void StartPaintFlash();

	// Ends the paint flash: the ball's skin materials back to tev mode 2
	void ResetPaintFlash();

	// Blob's colour changed: the paint flash starts at the next Update if Blob has paint
	void OnColourChanged();

	// Shows (the models of the current state) or hides Blob's models (m_iAnimFlags & 1)
	void SetVisible( TBOOL a_bVisible );

private:
	void PlayAnim( Anim* a_pAnim, TFLOAT a_fBlendInSpeed );
	// Plays the one-shot a_pAnim (not looping) while idle (state 3) and no other one plays
	TBOOL PlayOneShot( Anim* a_pAnim );
	void  SetEyeState( TINT a_iEyeState, TBOOL a_bRestart );
	void  ShowBallModels();
	void  ShowCharModel();
	void  StopCurrentAnim( TFLOAT a_fBlendOutSpeed );
	void  UpdateLevelStart();
	void  OnStateFinished();
	void  UpdateState0( TFLOAT a_fDeltaTime );
	void  UpdateState1( TFLOAT a_fDeltaTime );
	void  UpdateState2( TFLOAT a_fDeltaTime );
	void  UpdateState4( TFLOAT a_fDeltaTime );
	void  UpdateState5( TFLOAT a_fDeltaTime );
	void  UpdateState6( TFLOAT a_fDeltaTime );
	void  UpdateState3( TFLOAT a_fDeltaTime );
	void  UpdateState7( TFLOAT a_fDeltaTime );
	void  UpdateState8( TFLOAT a_fDeltaTime );
	void  UpdateState9( TFLOAT a_fDeltaTime );
	void  UpdateState10( TFLOAT a_fDeltaTime );
	void  UpdateState11( TFLOAT a_fDeltaTime );
	void  UpdateState12( TFLOAT a_fDeltaTime );
	void  UpdateState13( TFLOAT a_fDeltaTime );
	void  UpdateState14( TFLOAT a_fDeltaTime );
	void  UpdateState19( TFLOAT a_fDeltaTime );
	void  UpdateState20( TFLOAT a_fDeltaTime );
	void  UpdateState16( TFLOAT a_fDeltaTime );
	void  UpdateState21( TFLOAT a_fDeltaTime );
	// The shield states 0x17 (on), 0x18 (off) and 0x19 (on)
	void UpdateStateShield( TFLOAT a_fDeltaTime );
	void UpdateState30( TFLOAT a_fDeltaTime );
	// The states 0x1A..0x20 (except 0x1E): the animation is over, the next state
	void UpdateStateAnimFinished();
	// Whether the idle animations may play
	TBOOL IsIdleAllowed();
	TBOOL IsAnimPlaying( Anim* a_pAnim );
	TBOOL IsAnimFinished( Anim* a_pAnim );

	// The animations of the surface a_pSurface (an entity, type at 0x170) the blob is on
	static Anim* GetSurfaceAnims( void* a_pSurface );
	static Anim* GetSurfaceAnim2( void* a_pSurface );
	static Anim* GetSurfaceAnim5( void* a_pSurface );
	// A random animation of a_pSurface's group (SetState 3 after the idle)
	static Anim* GetSurfaceAnim3( void* a_pSurface );
	static Anim* GetSurfaceAnim6( void* a_pSurface );
	// The landmark animation of a_pSurface (SetState 0x1A, 0x1C)
	static Anim* GetSurfaceLandmarkAnim( void* a_pSurface );
	// A random animation of a_pSurface (SetState 0x1B, 0x1D)
	static Anim* GetSurfaceRandomAnim( void* a_pSurface );
	// A random animation of AnimTable's m_pAnims04 (SetState 0x1E)
	static Anim* GetRandomUnk04Anim();

	// Fades a_pTo in to a_fMax, then a_pFrom out to a_fMin, by a_fStep
	static void BlendModels( AModelInstance2* a_pFrom, AModelInstance2* a_pTo, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep );

public:
	Anim* GetCurrentAnim() const { return m_pCurrentAnim; }

	// The alpha m_fAlpha is blended to and its speed (ABlobVisionHud)
	void SetTargetAlpha( TFLOAT a_fTarget, TFLOAT a_fSpeed )
	{
		m_fTargetAlpha = a_fTarget;
		m_fAlphaSpeed  = a_fSpeed;
	}
	AModelInstance2* GetCharModelInstance() const { return m_pCharModelI; }
	AModelInstance2* GetBallBodyModelInstance() const { return m_pBallBodyModelI; }
	AModelInstance2* GetBallEarsModelInstance() const { return m_pBallEarsModelI; }
	AModelInstance2* GetCrownModelInstance() const { return m_pCrownModelI; }

	// Delays the m_pAnims[ 9 ] animation of SetState 3 by 3 seconds
	// $deBlob: FUNCTION 00755a00
	void SetIdleAnimDelay() { m_fIdleAnimDelay = 3.0f; }

	// Blinks for a_fTime (squashing a civilian)
	// $deBlob: FUNCTION 0076df00
	void StartBlink( TFLOAT a_fTime )
	{
		m_iAnimFlags |= 2;
		m_fBlinkTimer = a_fTime;
	}

private:
	ABlobEntity*      m_pOwner;           // 0x00
	TINT              m_iState;           // 0x04 the current animation state (0x1f checked by ABlobEntity::Tick)
	TUINT32           m_uiEyeState;       // 0x08
	TFLOAT            m_fUnk0C;           // 0x0C ABlobEntity::CommonProperties::m_fUnkB8
	Anim*             m_pCurrentAnim;     // 0x10 PlayAnim
	Anim*             m_pStateAnim;       // 0x14
	TUINT32           m_uiUnk18;          // 0x18
	TINT              m_iLookIndex;       // 0x1C
	TFLOAT            m_fIdleAnimDelay;   // 0x20 > 0: SetState 3 plays m_pAnims[ 9 ]
	TFLOAT            m_fCelebrateTimer;  // 0x24 > 0: SetState 3 plays a random idle
	TFLOAT            m_fUnk28;           // 0x28
	TFLOAT            m_fUnk2C;           // 0x2C
	TFLOAT            m_fBlinkTimer;      // 0x30 the blink timers (m_iAnimFlags & 2)
	TFLOAT            m_fLookTimer;       // 0x34
	TFLOAT            m_fAlpha;           // 0x38 1.0f
	TFLOAT            m_fTargetAlpha;     // 0x3C 1.0f
	TFLOAT            m_fAlphaSpeed;      // 0x40 1.0f
	TINT              m_iAnimFlags;       // 0x44 1
	TUINT32           m_uiUnk48;          // 0x48
	AModelInstance2*  m_pBallBodyModelI;  // 0x4C blobball_body(_mp).trb
	AModelInstance2*  m_pBallEarsModelI;  // 0x50 blobball_ears(_mp).trb
	AModelInstance2*  m_pCharModelI;      // 0x54 blob_bodya(_mp).trb
	AModelInstance2*  m_pShieldModelI;    // 0x58 mp_shield.trb (multiplayer)
	AModelInstance2*  m_pCrownModelI;     // 0x5C crown.trb (multiplayer)
	ASkinMaterialHAL* m_pShockedMaterial; // 0x60 skin material "Blob_Shocked"
	ASkinMaterialHAL* m_pZPassMaterial;   // 0x64 skin material "Blob_ZPass"
	TFLOAT            m_fPaintFlashTimer; // 0x68
	TBOOL             m_bUnk6C;           // 0x6C
	TFLOAT            m_fShieldTime;      // 0x70 the time in the shield states (0x17, 0x18)
	TINT              m_iCurShieldKey;    // 0x74 the key of s_afShieldOnScales/s_afShieldOffScales

public:
	static TBOOL                   ms_bShieldMaterialCreated;
	static AnimationProperties*    ms_pAnimationProperties;
	static Toshi::TSkeleton*       ms_pSkeletonChar;
	static TINT                    s_iLastLevelStartAnim; // the last level start animation (SetState 0xF)
	static const Toshi::TMatrix44* ms_pLeftEyeBoneTransform;
	static const Toshi::TMatrix44* ms_pRightEyeBoneTransform;
	static ASkinMaterialHAL*       ms_pLeftEyeMaterial;
	static ASkinMaterialHAL*       ms_pRightEyeMaterial;
	static TINT16                  ms_iBoneSBBase;
	static TINT16                  ms_iBoneSBBody;
	static TINT16                  ms_iBoneEffect1;
	static TINT16                  ms_iBoneEffect2;
	static TINT16                  ms_iAngeredEyes; // "Idle_Angered_Eyes"
};                                                  // 0x78
