#pragma once
#include "Toshi/Core/TQList.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/Render/TAnimation.h"
#include "Toshi/Render/ResourceName.h"

TOSHI_NAMESPACE_START

class TSkeletonSequenceBone
{
public:
	/**
		 * Returns lerp progress coefficient that should be used for animating
		 * @param a_iCurrentAnimProgress The current progress of animation from 0 to 65535
		 * @param a_rCurrentKeyIndex The index of current frame
		 * @param a_rLerpFromIndex The index of key that is being interpolated from
		 * @param a_rLerpToIndex The index of key to which previous is being interpolated to
		 */
	TFLOAT GetKeyPair(TINT a_iCurrentAnimProgress, TUINT& a_rCurrentKeyIndex, TUINT& a_rLerpFromIndex, TUINT& a_rLerpToIndex);

	__forceinline TUINT16* GetKey(size_t a_iKeyIndex) { return TREINTERPRETCAST(TUINT16*, a_iKeyIndex * m_iKeySize + TREINTERPRETCAST(uintptr_t, m_pData)); }

	__forceinline TUINT GetKeyCount() const { return m_iNumKeys; }

	__forceinline TBOOL IsTranslateAnimated() const { return m_eFlags & 1; }

	__forceinline TBOOL Is2() const { return m_eFlags & 2; }

	__forceinline TUINT8 GetFlags() const { return m_eFlags; }

private:
	TUINT8   m_eFlags;
	TUINT8   m_iKeySize;
	TUINT    m_iNumKeys;
	TUINT16* m_pData;
};

class TSkeletonSequence
{
public:
	enum Flag : TUINT
	{
		FLAG_Overlay = BITFIELD(0),
	};

	enum LOOPMODE : TINT
	{
		LOOPMODE_LOOP
	};

public:
	TBOOL IsLooping() const { return m_eLoopMode == LOOPMODE_LOOP; }
	TBOOL IsOverlay() const { return m_eFlags & FLAG_Overlay; }

	TSkeletonSequenceBone* GetBones() { return m_pSeqBones; }
	TSkeletonSequenceBone* GetBone(TINT a_iIndex) { return &m_pSeqBones[a_iIndex]; }

	TFLOAT       GetDuration() const { return m_fDuration; }
	const TCHAR* GetName() const { return m_szName; }
	TINT16       GetUnk2() const { return m_iUnk2; }
	TINT         GetUnk3() const { return m_iUnk3; }

private:
	const TCHAR*           m_szName;
	Flag                   m_eFlags;
	TINT16                 m_iUnk2;
	LOOPMODE               m_eLoopMode;
	TFLOAT                 m_fDuration;
	TSkeletonSequenceBone* m_pSeqBones;
	TINT                   m_iUnk3;
};

class TSkeletonBone
{
public:
	const TQuaternion& GetRotation() const { return m_Rotation; }
	const TMatrix44&   GetTransform() const { return m_Transform; }
	const TMatrix44&   GetTransformInv() const { return m_TransformInv; }
	const TVector3&    GetPosition() const { return m_Position; }
	const TCHAR*       GetName() const { return m_szName; }
	TINT               GetParentBone() const { return m_iParentBone; }

private:
	TQuaternion  m_Rotation;
	TMatrix44    m_Transform;
	TMatrix44    m_TransformInv;
	const TCHAR* m_szName;
	TINT16       m_iParentBone;
	TVector3     m_Position;
	TCHAR        pad[12];
};

class TSkeleton
{
public:
	using t_fnQuatLerp = void (*)(TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress);

	using QUATINTERP = TUINT32;
	enum QUATINTERP_
	{
		QUATINTERP_Default,
		QUATINTERP_Slerp,
		QUATINTERP_Nlerp
	};

public:
	TSkeleton();

	void  Delete();
	TBOOL Create(TUINT32 param_1);

	class TSkeletonInstance* CreateInstance(TBOOL a_bSetBasePose);

	t_fnQuatLerp GetQInterpFn() const { return m_fnQuatLerp; }
	void         SetQInterpFn(QUATINTERP a_eQuatInterp);

	TSkeletonBone* GetBone(const TCHAR* a_cBoneName, TUINT32 a_iLength) { return GetBone(GetBoneID(a_cBoneName, a_iLength)); }
	TINT           GetBoneID(const TCHAR* a_cBoneName, TUINT32 a_iLength);

	TKeyframeLibraryInstance& GetKeyLibraryInstance() { return m_KeyLibraryInstance; }
	TINT                      GetAnimationMaxCount() { return m_iAnimationMaxCount; }
	TSkeletonBone*            GetBone(TINT a_iBone) { return &m_pBones[a_iBone]; }
	TSkeletonBone*            GetBones() { return m_pBones; }
	TINT                      GetInstanceCount() const { return m_iInstanceCount; }
	TINT                      GetBoneCount() const { return m_iBoneCount; }
	TINT                      GetAutoBoneCount() const { return m_iBoneCount - m_iManualBoneCount; }

	TINT               GetSequenceID(const TCHAR* a_sSequenceName, TUINT32 a_iLength);
	TSkeletonSequence* GetSequences() { return m_SkeletonSequences; }
	TSkeletonSequence* GetSequence(TINT16 a_iSeqID)
	{
		TASSERT(a_iSeqID < m_iSequenceCount);
		return &m_SkeletonSequences[a_iSeqID];
	}
	TSkeletonSequence* GetSequence(const TCHAR* a_sSequenceName, TUINT32 a_iLength) { return GetSequence(GetSequenceID(a_sSequenceName, a_iLength)); }

	TINT16 GetSequenceCount() { return m_iSequenceCount; }

	TSkeleton& operator=(const TSkeleton& a_rSkeleton) { TIMPLEMENT(); }

private:
	TINT16                   m_iBoneCount;         // 0x00
	TINT16                   m_iManualBoneCount;   // 0x02
	TINT16                   m_iSequenceCount;     // 0x04
	TINT16                   m_iAnimationMaxCount; // 0x06
	TINT16                   m_iInstanceCount;     // 0x08
	TKeyframeLibraryInstance m_KeyLibraryInstance; // 0x0C
	TSkeletonBone*           m_pBones;             // 0x34
	TSkeletonSequence*       m_SkeletonSequences;  // 0x38
	t_fnQuatLerp             m_fnQuatLerp;         // 0x3C
};

using TSkeletonInstanceBone = TMatrix44;

class TSkeletonInstance
{
public:
	struct BoneCache
	{
		TQuaternion Rotation;
		TVector3    Position;
	};

	friend TSkeleton;

public:
	TSkeletonInstance() = default;

	void       UpdateTime(TFLOAT a_fDeltaTime);
	void       UpdateState(TBOOL a_bForceUpdate);
	TMatrix44* GetBoneTransformCurrent(TINT a_iBone, TMatrix44& a_rMatrix);

	TAnimation* AddAnimation(TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed) { return AddAnimationFull(a_iSequenceIndex, a_fDestWeight, a_fBlendInSpeed, 0.0f, 0); }

	TAnimation* AddAnimationFull(TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed, TAnimation::Flags a_eAnimFlags);
	void        RemoveAnimation(TAnimation* a_pAnimation, TFLOAT a_fBlendOutSpeed);

	void        SetStateFromBasePose();
	TAnimation* IsAnimating(TINT16 a_iSequenceIndex);

	TSkeleton* GetSkeleton() { return m_pSkeleton; }
	TINT       GetSequenceMaxUnk3() const { return m_iSequenceMaxUnk3; }
	void       SetSequenceMaxUnk3(TINT a_iSequenceMaxUnk3) { m_iSequenceMaxUnk3 = a_iSequenceMaxUnk3; }

public:
	inline static TMatrix44 g_aForwardMatrices[TANIMATION_MAXBONES];
	inline static BoneCache g_aBonesCaches[TANIMATION_MAXBONES];

private:
	TINT                   m_iFlags;
	TUINT32                m_iSize;
	TSkeleton*             m_pSkeleton;
	TINT16                 m_iBaseAnimationCount;
	TINT16                 m_iOverlayAnimationCount;
	TQList<TAnimation>     m_BaseAnimations;
	TQList<TAnimation>     m_OverlayAnimations;
	TQList<TAnimation>     m_FreeAnimations;
	TSkeletonInstanceBone* m_pBones;
	TAnimation*            m_pAnimations;
	TFLOAT                 m_fTotalWeight;
	TINT                   m_iLastUpdateTimeFrame;
	TINT                   m_iLastUpdateStateFrame;
	TINT                   m_iSequenceMaxUnk3;
};

TOSHI_NAMESPACE_END
