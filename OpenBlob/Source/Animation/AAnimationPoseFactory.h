#pragma once
#include "AStack.h"

#include <Toshi/TSingleton.h>

class AModel2;
class AAnimationPoseList;

//-----------------------------------------------------------------------------
// Purpose: the pose lists of the entity types, created by AEntityFramework::Startup and updated every frame by
// AApplication: once a list's model is loaded the sequence names of its poses are resolved
// (AAnimationPoseList::PatchSeqIDs).
//-----------------------------------------------------------------------------
class AAnimationPoseFactory
    : public Toshi::TSingleton<AAnimationPoseFactory>
{
public:
	static TCONSTEXPR_VAR TINT MAX_POSE_LISTS = 128;

	struct PoseList
	{
		AModel2*            pModel;         // 0x0
		AAnimationPoseList* pPoseList;      // 0x4
		TBOOL               bSeqIDsPatched; // 0x8 PatchSeqIDs was done with pModel's skeleton
	};                                      // 0xC

public:
	AAnimationPoseFactory();
	virtual ~AAnimationPoseFactory(); // +0x00

	// The pose list of the entity type a_szName (made and loaded the first time, for a_pModel)
	AAnimationPoseList* GetPoseList( const TCHAR* a_szName, AModel2* a_pModel );

	void Update();

private:
	AStack<PoseList*, MAX_POSE_LISTS> m_lPoseList; // 0x04
};                                                 // 0x208
