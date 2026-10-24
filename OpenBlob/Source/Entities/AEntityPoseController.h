#pragma once
#include "Entities/AEntityComponent2.h"

class AAnimationPose;
class AAnimationPoseList;
class AToshiAnimInterface2;

namespace Toshi
{
class TAnimation;
}

//-----------------------------------------------------------------------------
// Purpose: plays the pose/animation of an entity's current state: follows ABaseEntity::m_iState and starts the state's
// pose or animation ("<type>_PoseMap_Main") on the model's AToshiAnimInterface2, keeping a manual overlay animation.
//-----------------------------------------------------------------------------
class AEntityPoseController
    : public AEntityComponent2
{
public:
	enum POSETYPE
	{
		POSETYPE_Anim,
		POSETYPE_Pose,
		POSETYPE_None,
	};

	// A random choice of a state's animation
	struct AnimOption
	{
		TINT         iAnim;        // 0x00 resolved from szAnimName, -1 until then
		TFLOAT       fProbability; // 0x04 cumulative
		const TCHAR* szAnimName;   // 0x08
	};                             // 0x0C

	struct StatePose
	{
		TINT        iPose;       // 0x00 a pose of m_pPoseInformation, < 0 to pick an animation
		TFLOAT      fMult;       // 0x04 the speed
		TINT        iNumOptions; // 0x08
		AnimOption* pOptions;    // 0x0C
	};                           // 0x10

	struct PoseMap
	{
		TINT       iNumStates; // 0x00
		StatePose* pStates;    // 0x04
	};

public:
	AEntityPoseController( ABaseEntity* a_pEntity, AAnimationPoseList* a_pPoseInformation );
	virtual ~AEntityPoseController() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	TFLOAT             GetPoseLength();
	void               SetStateDirty();
	Toshi::TAnimation* PlayManualOverlayAnim( const TCHAR* a_szName, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed );
	void               StopManualOverlayAnim( TFLOAT a_fBlendOutSpeed );
	void               SetManualOverlayAnimWeight( TFLOAT a_fWeight );
	void               SetManualOverlayAnimTime( TFLOAT a_fTime );
	void               ResetAnimIDs();

	void     SetPoseForState( TINT a_iCurState, TINT a_iNewState );
	POSETYPE GetPoseForState( TINT a_State, TINT& a_rOutID, TFLOAT& a_rOutMult );

	TINT GetCurrentState() const { return m_iCurrentState; }

	// $deBlob: FUNCTION 0076df20
	void SetPoseMap( PoseMap* a_pPoseMap ) { m_pPoseMap = a_pPoseMap; }

	static PoseMap* ClonePoseMap( const PoseMap* a_pPoseMap );
	static void     DestroyPoseMap( PoseMap* a_pPoseMap );

private:
	void UpdateAnimLoop();
	void RestartManualOverlayAnim();
	void StopOldAnims( TINT a_iNewAnim, TFLOAT a_fBlendOutSpeed );
	void StopOldAnims( AAnimationPose* a_pNewPose, TFLOAT a_fBlendOutSpeed );

	AToshiAnimInterface2* GetAnimInterface() const;

	static TINT PickAnimOption( PoseMap* a_pPoseMap, TINT a_iState );

private:
	TINT                m_iCurrentState;     // 0x14 the entity state the pose was set for
	TINT                m_iCurrentPose;      // 0x18
	TINT                m_iCurrentAnim;      // 0x1C
	TFLOAT              m_CurrentMult;       // 0x20
	AAnimationPoseList* m_pPoseInformation;  // 0x24
	PoseMap*            m_pPoseMap;          // 0x28 "<type>_PoseMap_Main"
	TBOOL               m_bNoBlend;          // 0x2C
	TINT                m_ManualOverlayAnim; // 0x30
};                                           // 0x34
