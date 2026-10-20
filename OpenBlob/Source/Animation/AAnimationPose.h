#pragma once
#include <Toshi/TString8.h>

namespace Toshi
{
class TSkeleton;
}

//-----------------------------------------------------------------------------
// Purpose: a pose: the animations (sequences) a skeleton plays together (Source\Animation\AAnimationPose.h, data from
// the pose TRBs). Played by AToshiAnimInterface2::SetPose.
//-----------------------------------------------------------------------------
class AAnimationPose
{
public:
	struct Component
	{
		TFLOAT fWeight;    // 0x00
		TFLOAT fSpeed;     // 0x04
		TFLOAT fTime;      // 0x08
		TINT   iSequence;  // 0x0C the skeleton sequence, < 0 ends the pose (AAnimationPoseList::PatchSeqIDs)
		TCHAR* szSequence; // 0x10 its name
	};                     // 0x14

public:
	// $deBlob: FUNCTION 00574740
	Component& GetComponent( TINT a_iIndex )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_iPoseComponentCount );
		return m_pComponents[ a_iIndex ];
	}

	// $deBlob: FUNCTION 00574790
	TINT GetComponentCount() const { return m_iPoseComponentCount; }

private:
	friend class AAnimationPoseList;

	Component* m_pComponents;         // 0x00
	TINT       m_iPoseComponentCount; // 0x04
	TINT       m_iPoseID;             // 0x08 the pose TRB's (AAnimationPoseList's hash index)
};                                    // 0x0C

//-----------------------------------------------------------------------------
// Purpose: the poses of an entity type (AEntityPoseController::m_pPoseInformation): the "<type>_poses_Pose" symbol of
// the pose data (AEntityData) hashed by pose ID. Made by AAnimationPoseFactory::GetPoseList.
//-----------------------------------------------------------------------------
class AAnimationPoseList
{
public:
	static TCONSTEXPR_VAR TINT MAX_POSES = 64;

	// The pose data of an entity type
	struct TRBInfo
	{
		TINT            m_iNumPoses; // 0x0
		AAnimationPose* m_pPoses;    // 0x4
	};

public:
	AAnimationPoseList( const TCHAR* a_szName );
	~AAnimationPoseList();

	// Finds the pose data of the entity type and hashes it
	void Load();

	// $deBlob: FUNCTION 0056dfc0
	void Unload()
	{
		m_pTRBInfo = TNULL;
		Toshi::TUtil::MemClear( m_PoseHash, sizeof( m_PoseHash ) );
	}

	void CreatePoseHash();

	// Resolves the sequence names of the poses' components to a_pSkeleton's sequences
	void PatchSeqIDs( Toshi::TSkeleton* a_pSkeleton );

	// $deBlob: FUNCTION 0056e930
	const Toshi::TString8& GetName() const { return m_strName; }

	// $deBlob: FUNCTION 007c6160
	AAnimationPose* GetPose( TINT a_iID )
	{
		TASSERT( a_iID >= 0 && a_iID < MAX_POSES );
		return m_PoseHash[ a_iID ];
	}

private:
	TRBInfo*        m_pTRBInfo;              // 0x00
	TUINT32         m_uiUnk04;               // 0x04
	Toshi::TString8 m_strName;               // 0x08 the entity type
	AAnimationPose* m_PoseHash[ MAX_POSES ]; // 0x14
};                                           // 0x114
