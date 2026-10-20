#pragma once
#include "Toshi/TQList.h"
#include "Toshi/TSystem.h"

#include "Math/TQuaternion.h"
#include "Math/TMatrix44.h"
#include "Math/TSphere.h"
#include "Render/TAnimation.h"

#define TBONE_INVALID -1
#define TSEQ_INVALID  -1

TOSHI_NAMESPACE_START

class TSkeletonInstance;

//-----------------------------------------------------------------------------
// de Blob's skeleton data is loaded straight from the TRB ("Skeleton" symbol of the models), so the layouts of
// TSkeleton, TSkeletonBone, TSkeletonSequence and TSkeletonSequenceBone are retail's.
//-----------------------------------------------------------------------------
class TSkeletonSequenceBone
{
public:
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE               = 0,
		FLAGS_TRANSLATE_ANIMATED = BITFLAG( 0 ),
		FLAGS_OVERLAY_ANIMATED   = BITFLAG( 1 ), // the bone is skipped by overlay animations
	};

public:
	/**
		 * Returns lerp progress coefficient that should be used for animating
		 * @param a_uiCurrentAnimProgress The current progress of animation from 0 to 65535
		 * @param a_rCurrentKeyIndex The index of current frame
		 * @param a_rLerpFromIndex The index of key that is being interpolated from
		 * @param a_rLerpToIndex The index of key to which previous is being interpolated to
		 */
	TFLOAT GetKeyPair( TUINT16 a_uiCurrentAnimProgress, TUINT16& a_rCurrentKeyIndex, TUINT16& a_rLerpFromIndex, TUINT16& a_rLerpToIndex );

	// Slerps the rotation (and lerps the translation if it's animated) of the bone between two keys
	void IntraBlend( TKeyframeLibraryInstance* a_pKeyLib, TFLOAT a_fProgress, TUINT16& a_rFromKey, TUINT16& a_rToKey, TQuaternion& a_rOutRotation, TVector4& a_rOutPosition );

	TFORCEINLINE TUINT16* GetKey( size_t a_iKeyIndex )
	{
		return TREINTERPRETCAST( TUINT16*, a_iKeyIndex * m_iKeySize + TREINTERPRETCAST( TUINTPTR, m_pData ) );
	}

	TFORCEINLINE TUINT16 GetKeyCount() const
	{
		return m_iNumKeys;
	}

	TFORCEINLINE TBOOL IsTranslateAnimated() const
	{
		return m_eFlags & FLAGS_TRANSLATE_ANIMATED;
	}

	TFORCEINLINE TBOOL IsOverlayAnimated() const
	{
		return m_eFlags & FLAGS_OVERLAY_ANIMATED;
	}

	TFORCEINLINE FLAGS GetFlags() const
	{
		return m_eFlags;
	}

public:
	FLAGS   m_eFlags;   // 0x00
	TUINT8  m_iKeySize; // 0x01
	TUINT16 m_iNumKeys; // 0x02
	TBYTE*  m_pData;    // 0x04 keys: TUINT16 time, quaternion index, translation index...
}; // 0x08

class TSkeletonSequence
{
public:
	enum MODE : TINT16
	{
		MODE_LOOPED,
		MODE_CLAMPED
	};

	enum Flag : TUINT16
	{
		FLAG_NONE    = 0,
		FLAG_OVERLAY = BITFLAG( 0 ),
	};

public:
	TBOOL IsBase() const { return !( m_eFlags & FLAG_OVERLAY ); }
	TBOOL IsOverlay() const { return ( m_eFlags & FLAG_OVERLAY ); }

	TSkeletonSequenceBone* GetBones() { return m_pSeqBones; }
	TSkeletonSequenceBone* GetBone( TINT a_iIndex ) { return &m_pSeqBones[ a_iIndex ]; }

	TFLOAT       GetDuration() const { return m_fDuration; }
	const TCHAR* GetName() const { return m_szName; }
	MODE         GetMode() const { return m_eMode; }
	TINT         GetBoundingSphereIndex() const { return m_iBoundingSphereIndex; }

public:
	const TCHAR*           m_szName;    // 0x00
	Flag                   m_eFlags;    // 0x04
	MODE                   m_eMode;     // 0x06
	TINT                   m_iUnk2;     // 0x08 not read by the engine
	TFLOAT                 m_fDuration; // 0x0C
	TSkeletonSequenceBone* m_pSeqBones; // 0x10
	TINT                   m_iBoundingSphereIndex; // 0x14 the animation bounding sphere of the sequence (TAnimation::FindMaxBoundingSphereIndex)
}; // 0x18

class TSkeletonBone
{
public:
	TQuaternion&       GetRotation() { return m_Rotation; }
	const TQuaternion& GetRotation() const { return m_Rotation; }

	TMatrix44&       GetTransform() { return m_Transform; }
	const TMatrix44& GetTransform() const { return m_Transform; }

	TMatrix44&      GetTransformInv() { return m_TransformInv; }
	const TVector3& GetPosition() const { return m_Position; }

	const TCHAR* GetName() const { return m_szName; }

	TINT GetParentBone() const { return m_iParentBone; }

public:
	TQuaternion  m_Rotation;     // 0x00
	TMatrix44    m_Transform;    // 0x10
	TMatrix44    m_TransformInv; // 0x50
	const TCHAR* m_szName;       // 0x90
	TINT16       m_iParentBone;  // 0x94
	TVector3     m_Position;     // 0x98
	TBYTE        m_aUnk[ 12 ];   // 0xA4
}; // 0xB0

TSTATICASSERT( sizeof( TSkeletonBone ) == 0xB0 );
TSTATICASSERT( sizeof( TSkeletonSequence ) == 0x18 );

class TSkeleton
{
public:
	using t_fnQuatLerp = void ( * )( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress );

	using QUATINTERP = TUINT32;
	enum QUATINTERP_
	{
		QUATINTERP_Default,
		QUATINTERP_Slerp,
		QUATINTERP_Nlerp
	};

	friend TSkeletonInstance;

public:
	TSkeleton();
	~TSkeleton();

	TSkeletonInstance* CreateInstance( TBOOL a_bSetBasePose );

	t_fnQuatLerp GetQInterpFn() const { return m_fnQuatLerp; }
	void         SetQInterpFn( QUATINTERP a_eQuatInterp );

	// NOTE: a_iLength is passed on to T2String8::CompareNoCase, -1 compares the whole names
	TSkeletonBone* GetBone( const TCHAR* a_cBoneName, TINT a_iLength = -1 ) { return GetBone( GetBoneID( a_cBoneName, a_iLength ) ); }
	TINT16         GetBoneID( const TCHAR* a_cBoneName, TINT a_iLength = -1 );

	TKeyframeLibraryInstance& GetKeyLibraryInstance() { return m_KeyLibraryInstance; }
	// $deBlob: FUNCTION 0068fae0
	TINT16                    GetAnimationMaxCount() { return m_iAnimationMaxCount; }

	// Limits the number of animations an instance can play (at most the sequence count), returns the previous count
	TINT16 SetAnimationMaxCount( TINT16 a_iAnimationMaxCount );

	// $deBlob: FUNCTION 0068fb10
	TSkeletonBone* GetBone( TINT a_iBone )
	{
		TASSERT( a_iBone >= 0 && a_iBone < m_iBoneCount );
		return &m_pBones[ a_iBone ];
	}

	TSkeletonBone* GetBones() { return m_pBones; }
	TINT           GetInstanceCount() const { return m_iInstanceCount; }
	TINT           GetBoneCount() const { return m_iBoneCount; }
	TINT           GetAutoBoneCount() const { return m_iBoneCount - m_iManualBoneCount; }

	TINT16             GetSequenceID( const TCHAR* a_sSequenceName, TINT a_iLength = -1 );
	TSkeletonSequence* GetSequences() { return m_SkeletonSequences; }

	// $deBlob: FUNCTION 00560fc0
	TSkeletonSequence* GetSequence( TINT a_iSeqID )
	{
		TASSERT( a_iSeqID < m_iSequenceCount );
		return &m_SkeletonSequences[ a_iSeqID ];
	}

	TSkeletonSequence* GetSequence( const TCHAR* a_sSequenceName, TINT a_iLength = -1 ) { return GetSequence( GetSequenceID( a_sSequenceName, a_iLength ) ); }

	TINT16 GetSequenceCount() { return m_iSequenceCount; }

	// $deBlob: FUNCTION 00572d00
	TSphere* GetAnimationBoundingSphere( TUINT32 a_index )
	{
		TASSERT( a_index < m_animationBoundingSpheresCount );
		return &m_animationBoundingSpheres[ a_index ];
	}

	// Transform of a_iBone relative to its parent in a_iSequence at a_fTime, a_uiCurrentKey is the key to start the search from.
	// Returns the current key.
	TUINT16 GetBoneTransformLocal( TINT a_iBone, TINT a_iSequence, TFLOAT a_fTime, TUINT16 a_uiCurrentKey, TMatrix44& a_rOutTransform );

	// Transform of a_iBone in the space of the skeleton in a_iSequence at a_fTime
	void GetBoneTransform( TINT a_iBone, TINT a_iSequence, TFLOAT a_fTime, TMatrix44& a_rOutTransform );

	// Walks the bones, sequences and (a_bDumpKeys) keys through the asserting accessors
	void DebugDump( TBOOL a_bDumpKeys );

public:
	TINT16                   m_iBoneCount;         // 0x00
	TINT16                   m_iManualBoneCount;   // 0x02
	TINT16                   m_iSequenceCount;     // 0x04
	TINT16                   m_iAnimationMaxCount; // 0x06
	TINT16                   m_iInstanceCount;     // 0x08
	TKeyframeLibraryInstance m_KeyLibraryInstance; // 0x0C
	TSkeletonBone*           m_pBones;             // 0x34
	TSkeletonSequence*       m_SkeletonSequences;  // 0x38

	union // 0x3C
	{
		t_fnQuatLerp m_fnQuatLerp;
		QUATINTERP   m_eQuatLerpType;
	};

	// NOTE: the bounding spheres come with the TRB data (the ctor doesn't clear them)
	TSphere* m_animationBoundingSpheres;      // 0x40
	TUINT32  m_animationBoundingSpheresCount; // 0x44
}; // 0x48

TSTATICASSERT( sizeof( TSkeleton ) == 0x48 );

// NOTE: de Blob keeps only the matrix per bone (Barnyard also keeps the rotation and the position), the blended
// rotation and position are built in TSkeletonInstance::g_aBoneStates by UpdateState
struct TSkeletonInstanceBone
{
	TMatrix44 m_Transform;
}; // 0x40

class TSkeletonInstance
{
public:
	friend TSkeleton;

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE         = 0,
		FLAGS_UPDATE_STATE = BITFLAG( 0 ), // the state has to be updated on the next UpdateTime
	};

	// The blended state of a bone (g_aBoneStates)
	struct BoneState
	{
		TQuaternion m_Rotation; // 0x00
		TVector3    m_Position; // 0x10
	}; // 0x1C

public:
	TSkeletonInstance();
	~TSkeletonInstance();

	TAnimation* AddAnimation( TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed );
	TAnimation* AddManagedAnimation( TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed );
	TAnimation* AddAnimationFull( TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed, TUINT32 a_uiFlags );

	// Copies the playing animations of a_pSrc (the instances must share the skeleton)
	void CopyAnimations( TSkeletonInstance* a_pSrc );

	// NOTE: retail's CopyAnimation doesn't use this, it's __stdcall with two arguments
	static void __stdcall CopyAnimation( TAnimation* a_pDst, TAnimation* a_pSrc );

	TAnimation* IsAnimating( TINT16 a_iSeqId );

	void RemoveAnimation( TAnimation* a_pAnimation, TFLOAT a_fBlendOutSpeed );
	void RemoveAllAnimations( TFLOAT a_fBlendOutSpeed = 0.0f );

	// NOTE: Barnyard's, de Blob has no such function
	void RemoveAllBaseAnimations();

	void MoveAnimationToFront( TAnimation* a_pAnimation );
	void MoveAnimationToBack( TAnimation* a_pAnimation );

	// $deBlob: FUNCTION 0068dca0
	TAnimation* GetFreeAnimation() { return m_FreeAnimations.IsEmpty() ? TNULL : &*m_FreeAnimations.Begin(); }

	void UpdateTime( TFLOAT a_fDeltaTime );
	void UpdateState( TBOOL a_bForceUpdate );

	// Marks the state as up to date for the current frame without recalculating it
	void UpdateStateFrame();

	const TMatrix44& GetBoneTransformCurrent( TINT a_iBone );
	TMatrix44&       GetBoneTransformCurrent( TINT a_iBone, TMatrix44& a_rOutTransform );
	void             SetStateFromBasePose();

	void Delete();

	TSkeleton* GetSkeleton() { return m_pSkeleton; }

	// The animation bounding sphere of the playing animations (m_iMaxBoundingSphereIndex), TNULL without one
	// $deBlob: FUNCTION 00572d60
	TSphere* GetAnimationBoundingSphere()
	{
		if ( m_iMaxBoundingSphereIndex < 0 )
			return TNULL;

		return m_pSkeleton->GetAnimationBoundingSphere( m_iMaxBoundingSphereIndex );
	}

	// $deBlob: FUNCTION 0068cf40
	// $Barnyard: FUNCTION 006c9f50
	TINT GetBoneID( const TCHAR* a_szBoneName ) { return m_pSkeleton->GetBoneID( a_szBoneName, -1 ); }

	const TSkeletonInstanceBone* GetBones() { return m_pBones; }
	const TSkeletonInstanceBone& GetBone( TINT a_uiIndex );

	TBOOL               IsAnyBaseAnimationPlaying() const { return !m_BaseAnimations.IsEmpty(); }
	TQList<TAnimation>& GetBaseAnimations() { return m_BaseAnimations; }
	TBOOL               IsAnyOverlayAnimationPlaying() const { return !m_OverlayAnimations.IsEmpty(); }
	TBOOL               IsAnyAnimationPlaying() const { return !m_BaseAnimations.IsEmpty() || !m_OverlayAnimations.IsEmpty(); }
	TINT16              GetBaseAnimationCount() const { return m_iBaseAnimationCount; }
	TINT16              GetOverlayAnimationCount() const { return m_iOverlayAnimationCount; }

	FLAGS GetFlags() const { return m_eFlags; }
	void  SetFlags( FLAGS a_eFlags ) { m_eFlags = a_eFlags; }

	TINT GetMaxBoundingSphereIndex() const { return m_iMaxBoundingSphereIndex; }

	// Recalculates m_iMaxBoundingSphereIndex from the playing animations
	// $deBlob: FUNCTION 0068dd40
	void UpdateMaxBoundingSphereIndex()
	{
		m_iMaxBoundingSphereIndex = TAnimation::FindMaxBoundingSphereIndex( m_BaseAnimations );
		m_iMaxBoundingSphereIndex = TMath::Max( TAnimation::FindMaxBoundingSphereIndex( m_OverlayAnimations ), m_iMaxBoundingSphereIndex );
	}

public:
	static TMatrix44 g_aForwardMatrices[ TANIMATION_MAXBONES ];
	static BoneState g_aBoneStates[ TANIMATION_MAXBONES ];

private:
	FLAGS                  m_eFlags;                 // 0x00
	TUINT32                m_iSize;                  // 0x04
	TSkeleton*             m_pSkeleton;              // 0x08
	TINT16                 m_iBaseAnimationCount;    // 0x0C
	TINT16                 m_iOverlayAnimationCount; // 0x0E
	TQList<TAnimation>     m_BaseAnimations;         // 0x10
	TQList<TAnimation>     m_OverlayAnimations;      // 0x18
	TQList<TAnimation>     m_FreeAnimations;         // 0x20
	TSkeletonInstanceBone* m_pBones;                 // 0x28
	TAnimation*            m_pAnimations;            // 0x2C
	TFLOAT                 m_fTotalWeight;           // 0x30
	TINT                   m_iLastUpdateTimeFrame;   // 0x34
	TINT                   m_iLastUpdateStateFrame;  // 0x38
	TINT                   m_iMaxBoundingSphereIndex; // 0x3C max TSkeletonSequence::m_iBoundingSphereIndex of the playing animations
}; // 0x40, followed by the bones and the animations

TSTATICASSERT( sizeof( TSkeletonInstance ) == 0x40 );
TSTATICASSERT( sizeof( TSkeletonInstance::BoneState ) == 0x1C );

TOSHI_NAMESPACE_END
