#pragma once
#include "Toshi/Core/TQList.h"
#include "Toshi/File/TTRB.h"

#undef LoadLibrary

TOSHI_NAMESPACE_START

extern TMemoryHeap* s_pAnimMemHeap;
constexpr TINT      TANIMATION_MAXBONES = 128;

using TAnimVector     = TVector3;
using TAnimQuaternion = TQuaternion;
using TAnimScale      = TFLOAT;
using TAnimationBone  = TUINT;

class TSkeletonInstance;

class TAnimation : public TQList<TAnimation>::TNode
{
public:
	using Flags = TUINT16;
	enum Flags_ : Flags
	{
		Flags_None                = 0,
		Flags_Active              = BITFIELD(0),
		Flags_Managed             = BITFIELD(1),
		Flags_Unknown2            = BITFIELD(2),
		Flags_UpdateStateOnRemove = BITFIELD(3),
	};

	using Mode = TUINT32;
	enum Mode_ : Mode
	{
		MODE_UNK1,
		MODE_UNK2,
		MODE_UNK3,
	};

public:
	TAnimation() = default;

	TBOOL UpdateTime(TFLOAT a_fDeltaTime);
	void  RemoveAnimation(TFLOAT a_fBlendOutSpeed);

	void   SetMode(Mode a_eMode) { m_eMode = a_eMode; }
	void   SetSkeletonInstance(TSkeletonInstance* a_pSkeletonInstance) { m_pSkeletonInstance = a_pSkeletonInstance; }
	TFLOAT SetSpeed(TFLOAT a_fSpeed) { return std::exchange(m_fSpeed, a_fSpeed); }
	TFLOAT SetDestWeight(TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed);
	void   SetDestWeightExplicit(TFLOAT a_fDestWeight) { m_fDestWeight = a_fDestWeight; }
	void   SetBlendInSpeed(TFLOAT a_fBlendInSpeed) { m_fBlendInSpeed = a_fBlendInSpeed; }
	void   SetBlendOutSpeed(TFLOAT a_fBlendOutSpeed) { m_fBlendOutSpeed = a_fBlendOutSpeed; }
	void   SetWeight(TFLOAT a_fWeight) { m_fWeight = a_fWeight; }
	void   SetFlags(Flags a_eFlags) { m_eFlags = a_eFlags; }
	void   SetUnk3(TINT16 a_iUnk3) { m_iUnk3 = a_iUnk3; }
	void   SetSequence(TUINT a_iSeqID) { m_iSeqID = a_iSeqID; }

	void SetUpdateStateOnRemove(TBOOL a_bEnable)
	{
		if (a_bEnable)
			m_eFlags |= Flags_UpdateStateOnRemove;
		else
			m_eFlags &= ~Flags_UpdateStateOnRemove;
	}

	void ResetTime()
	{
		m_fTotalTime = 0.0f;
		m_fSeqTime   = 0.0f;
	}

	Flags GetFlags() const { return m_eFlags; }
	Mode  GetMode() const { return m_eMode; }
	TBOOL IsActive() const { return m_eFlags & Flags_Active; }
	TBOOL IsManaged() const { return m_eFlags & Flags_Managed; }
	TBOOL IsUpdateStateOnRemove() const { return m_eFlags & Flags_UpdateStateOnRemove; }

	TUINT                    GetSequence() const { return m_iSeqID; }
	class TSkeletonSequence* GetSequencePtr() const;
	TSkeletonInstance*       GetSkeletonInstance() const { return m_pSkeletonInstance; }
	TFLOAT                   GetSpeed() const { return m_fSpeed; }
	TFLOAT                   GetSeqTime() const { return m_fSeqTime; }
	TFLOAT                   GetTotalTime() const { return m_fTotalTime; }
	TFLOAT                   GetWeight() const { return m_fWeight; }
	TFLOAT                   GetDestWeight() const { return m_fDestWeight; }
	TFLOAT                   GetBlendInSpeed() const { return m_fBlendInSpeed; }
	TFLOAT                   GetBlendOutSpeed() const { return m_fBlendOutSpeed; }

	TFLOAT& GetWeight() { return m_fWeight; }

	TAnimationBone* GetBones() { return TREINTERPRETCAST(TAnimationBone*, this + 1); }
	TAnimationBone* GetBone(TINT a_iIndex) { return &TREINTERPRETCAST(TAnimationBone*, this + 1)[a_iIndex]; }

public:
	static TINT FindSequenceMaxUnk3(const TQList<TAnimation>& a_rList);

private:
	TSkeletonInstance* m_pSkeletonInstance;
	TUINT              m_iSeqID;
	Flags              m_eFlags;
	TINT16             m_iUnk3;
	Mode               m_eMode;
	TFLOAT             m_fSpeed;
	TFLOAT             m_fWeight;
	TFLOAT             m_fDestWeight;
	TFLOAT             m_fTotalTime;
	TFLOAT             m_fSeqTime;
	TFLOAT             m_fBlendInSpeed;
	TFLOAT             m_fBlendOutSpeed;
};

class TKeyframeLibrary : public TDList<TKeyframeLibrary>::TNode
{
public:
	friend class TKeyframeLibraryInstance;

	struct TRBHeader
	{
		const TCHAR*     m_szName;
		TVector3         m_SomeVector;
		TINT             m_iNumTranslations;
		TINT             m_iNumQuaternions;
		TINT             m_iNumScales;
		TINT             m_iTranslationSize;
		TINT             m_iQuaternionSize;
		TINT             m_iScaleSize;
		TAnimVector*     m_pTranslations;
		TAnimQuaternion* m_pQuaternions;
		TAnimScale*      m_pScales;
	};

public:
	static TKeyframeLibrary* Create(TRBHeader* a_pTRBHeader);

	void Delete();

	const TCHAR* GetName() const { return m_szName; }

private:
	TKeyframeLibrary();

	void IncreaseRefCount() { m_iReferenceCount++; }

	void DecreaseRefCount() { m_iReferenceCount--; }

private:
	const TCHAR*     m_szName;
	TVector3         m_SomeVector;
	TINT             m_iReferenceCount;
	TINT             m_iNumTranslations;
	TINT             m_iNumQuaternions;
	TINT             m_iNumScales;
	TINT             m_iTranslationSize;
	TINT             m_iQuaternionSize;
	TINT             m_iScaleSize;
	TAnimVector*     m_pTranslations;
	TAnimQuaternion* m_pQuaternions;
	TAnimScale*      m_pScales;
	TTRB*            m_pTRB;
};

class TKeyframeLibraryInstance
{
public:
	TKeyframeLibraryInstance();

	TKeyframeLibrary* Create(TKeyframeLibrary* a_pLibrary);
	TKeyframeLibrary* CreateEx(TKeyframeLibrary* a_pLibrary, TINT a_iTKeyCount, TINT a_iQKeyCount, TINT a_iSKeyCount, TINT a_iTBaseIndex, TINT a_iQBaseIndex, TINT a_iSBaseIndex);

	void Destroy();

	TAnimVector* GetT(TINT a_iIndex) const
	{
		TASSERT(a_iIndex < m_iTKeyCount);
		return &m_pTranslations[a_iIndex];
	}

	TAnimQuaternion* GetQ(TINT a_iIndex) const
	{
		TASSERT(a_iIndex < m_iQKeyCount);
		return &m_pQuaternions[a_iIndex];
	}

	TAnimScale GetS(TINT a_iIndex) const
	{
		TASSERT(a_iIndex < m_iQKeyCount);
		return m_pScales[a_iIndex];
	}

	void SetLibrary(TKeyframeLibrary* a_pLibrary) { m_pLibrary = a_pLibrary; }
	void SetTCount(TINT a_iTCount) { m_iTKeyCount = a_iTCount; }
	void SetQCount(TINT a_iQCount) { m_iQKeyCount = a_iQCount; }
	void SetSCount(TINT a_iSCount) { m_iSKeyCount = a_iSCount; }

	TKeyframeLibrary* GetLibrary() const { return m_pLibrary; }
	TINT              GetTCount() const { return m_iTKeyCount; }
	TINT              GetQCount() const { return m_iQKeyCount; }
	TINT              GetSCount() const { return m_iSKeyCount; }

private:
	TKeyframeLibrary* m_pLibrary;
	TINT              m_iTBaseIndex;
	TINT              m_iQBaseIndex;
	TINT              m_iSBaseIndex;
	TINT              m_iTKeyCount;
	TINT              m_iQKeyCount;
	TINT              m_iSKeyCount;
	TAnimVector*      m_pTranslations;
	TAnimQuaternion*  m_pQuaternions;
	TAnimScale*       m_pScales;
};

class TKeyframeLibraryManager
{
public:
	TKeyframeLibraryManager()  = default;
	~TKeyframeLibraryManager() = default;

	TKeyframeLibrary* GetLibrary(const TCHAR* a_szName);

	TKeyframeLibrary* LoadLibrary(TKeyframeLibrary::TRBHeader* a_pTRBHeader);
	TKeyframeLibrary* LoadLibraryFromTRB(TTRB* a_pTRB, const TCHAR* a_szSymbolName);
	void              UnloadLibrary(TKeyframeLibrary* a_pLibrary);

private:
	TDList<TKeyframeLibrary> m_List;
	TINT                     m_iNumLibraries = 0;
};

TOSHI_NAMESPACE_END
