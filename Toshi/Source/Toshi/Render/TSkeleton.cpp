#include "ToshiPCH.h"
#include "TSkeleton.h"

TOSHI_NAMESPACE_START

TSkeleton::TSkeleton()
{
	m_iBoneCount         = 0;
	m_iManualBoneCount   = 0;
	m_iSequenceCount     = 0;
	m_iAnimationMaxCount = 0;
	m_iInstanceCount     = 0;
	m_pBones             = TNULL;
	m_SkeletonSequences  = TNULL;
}

void TSkeleton::Delete()
{
	if (m_KeyLibraryInstance.GetLibrary() != TNULL)
	{
		auto& instance = GetKeyLibraryInstance();
		instance.SetSCount(instance.GetSCount() - 1);
		instance.SetLibrary(TNULL);
	}

	//tfree(this);
}

TBOOL TSkeleton::Create(TUINT32 param_1)
{
	TIMPLEMENT();
	return TFALSE;
}

// $deBlob: FUNCTION 0068ed60
TSkeletonInstance* TSkeleton::CreateInstance(TBOOL a_bSetBasePose)
{
	if (TINT(m_fnQuatLerp) < 3)
	{
		SetQInterpFn(TREINTERPRETCAST(QUATINTERP, m_fnQuatLerp));
	}

	auto               iAutoBoneCount = GetAutoBoneCount();
	size_t             iAnimationSize = iAutoBoneCount * sizeof(TAnimationBone) + TMath::AlignNum(sizeof(TAnimation));
	size_t             iInstanceSize  = sizeof(TSkeletonInstance) + sizeof(TSkeletonInstanceBone) * iAutoBoneCount + iAnimationSize * GetAnimationMaxCount();
	TSkeletonInstance* pInstance;

	if (s_pAnimMemHeap)
	{
		pInstance = TSTATICCAST(TSkeletonInstance, s_pAnimMemHeap->Malloc(iInstanceSize));
	}
	else
	{
		pInstance = TSTATICCAST(TSkeletonInstance, TMalloc(iInstanceSize));
	}

	new (pInstance) TSkeletonInstance();
	pInstance->m_pSkeleton              = this;
	pInstance->m_iSize                  = iInstanceSize;
	pInstance->m_iBaseAnimationCount    = 0;
	pInstance->m_iOverlayAnimationCount = 0;
	pInstance->m_iFlags                 = 0;
	pInstance->m_pBones                 = TREINTERPRETCAST(TSkeletonInstanceBone*, pInstance + 1);
	pInstance->m_pAnimations            = TREINTERPRETCAST(TAnimation*, pInstance->m_pBones + iAutoBoneCount);
	pInstance->m_fTotalWeight           = 0.0f;
	pInstance->m_iLastUpdateStateFrame  = 0;
	pInstance->m_iLastUpdateTimeFrame   = 0;
	pInstance->m_iSequenceMaxUnk3       = -1;

	for (TINT i = 0; i < GetAnimationMaxCount(); i++)
	{
		TAnimation* pAnimation = TREINTERPRETCAST(TAnimation*, TREINTERPRETCAST(uintptr_t, pInstance->m_pAnimations) + i * iAnimationSize);

		new (pAnimation) TAnimation();
		pInstance->m_FreeAnimations.PushFront(pAnimation);
	}

	if (a_bSetBasePose) pInstance->SetStateFromBasePose();

	m_iInstanceCount++;
	return pInstance;
}

// $deBlob: FUNCTION 0068f480
void TSkeleton::SetQInterpFn(QUATINTERP a_eQuatInterp)
{
	if (a_eQuatInterp == QUATINTERP_Slerp)
		m_fnQuatLerp = TQuaternion::Slerp;
	else
		m_fnQuatLerp = TQuaternion::Nlerp;
}

// $deBlob: FUNCTION 0068ee90
TINT TSkeleton::GetBoneID(const TCHAR* a_cBoneName, TUINT32 a_iLength)
{
	for (TINT16 i = 0; i < m_iBoneCount; i++)
	{
		if (TStringManager::String8CompareNoCase(m_pBones[i].GetName(), a_cBoneName, a_iLength) == 0)
		{
			return i;
		}
	}

	return -1;
}

// $deBlob: FUNCTION 0068eee0
TINT TSkeleton::GetSequenceID(const TCHAR* a_sSequenceName, TUINT32 a_iLength)
{
	for (TINT16 i = 0; i < m_iSequenceCount; i++)
	{
		if (TStringManager::String8CompareNoCase(m_SkeletonSequences[i].GetName(), a_sSequenceName, a_iLength) == 0)
		{
			return i;
		}
	}

	return -1;
}

// $deBlob: FUNCTION 0068cf60
void TSkeletonInstance::UpdateTime(TFLOAT a_fDeltaTime)
{
	if (!m_BaseAnimations.IsEmpty() || (!m_OverlayAnimations.IsEmpty() && m_iLastUpdateTimeFrame != TSystemManager::GetSingleton()->GetFrameCount()))
	{
		m_iLastUpdateTimeFrame = TSystemManager::GetSingleton()->GetFrameCount();
		m_fTotalWeight         = 0.0f;

		// Update base animations
		for (auto pAnim = m_BaseAnimations.Begin(); pAnim != m_BaseAnimations.End(); pAnim++)
		{
			if (pAnim->UpdateTime(a_fDeltaTime))
			{
				m_pSkeleton->GetSequence(pAnim->GetSequence());
				m_fTotalWeight += pAnim->GetWeight();
			}
		}

		// Update overlay animations
		for (auto pAnim = m_OverlayAnimations.Begin(); pAnim != m_OverlayAnimations.End(); pAnim++)
		{
			pAnim->UpdateTime(a_fDeltaTime);
		}

		if (HASFLAG(m_iFlags & 1))
		{
			m_iFlags &= ~1;
			UpdateState(TFALSE);
		}
	}
}

// $deBlob: FUNCTION 0068d0e0
void TSkeletonInstance::UpdateState(TBOOL a_bForceUpdate)
{
	if ((a_bForceUpdate || m_iLastUpdateStateFrame != TSystemManager::GetSingleton()->GetFrameCount()) && m_pSkeleton->GetKeyLibraryInstance().GetLibrary() != TNULL)
	{
		m_iLastUpdateStateFrame = TSystemManager::GetSingleton()->GetFrameCount();

		const auto QInterpFn           = m_pSkeleton->GetQInterpFn();
		TFLOAT     fOneOverTotalWeight = 1.0f;

		if (1.0f < m_fTotalWeight) fOneOverTotalWeight = 1.0f / m_fTotalWeight;

		TASSERT(m_pSkeleton->GetAutoBoneCount() < TANIMATION_MAXBONES);

		for (TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++)
		{
			TASSERT(i < TANIMATION_MAXBONES);

			auto& rBoneCache = g_aBonesCaches[i];
			auto  pBone      = m_pSkeleton->GetBone(i);

			if (m_BaseAnimations.IsEmpty())
			{
				rBoneCache.Rotation = pBone->GetRotation();
				rBoneCache.Position = pBone->GetPosition();
			}
			else
			{
				TFLOAT fWeightTotalRatio = 0.0f;
				TBOOL  bBoneHasState     = TFALSE;

				for (auto it = m_BaseAnimations.Begin(); it != m_BaseAnimations.End(); it++)
				{
					auto pSeq     = m_pSkeleton->GetSequence(it->GetSequence());
					auto pSeqBone = pSeq->GetBone(i);

					TINT iCurrentKeyframePos = TINT((it->GetSeqTime() / pSeq->GetDuration()) * 65535);

					TUINT  iLerpFromIndex;
					TUINT  iLerpToIndex;
					TFLOAT fLerpProgress = pSeqBone->GetKeyPair(iCurrentKeyframePos, *it->GetBone(i), iLerpFromIndex, iLerpToIndex);

					TFLOAT fWeightRatio = it->GetWeight() * fOneOverTotalWeight;

					if (fWeightRatio > 0.0f && pSeqBone->GetKeyCount() != 0)
					{
						auto pFromKey = pSeqBone->GetKey(iLerpFromIndex);
						auto pToKey   = pSeqBone->GetKey(iLerpToIndex);

						auto& rKeyLibrary = m_pSkeleton->GetKeyLibraryInstance();
						auto  pFromQuat   = rKeyLibrary.GetQ(pFromKey[1]);
						auto  pToQuat     = rKeyLibrary.GetQ(pToKey[1]);

						if (it == m_BaseAnimations.Head())
						{
							bBoneHasState     = TTRUE;
							fWeightTotalRatio = fWeightRatio;
							QInterpFn(rBoneCache.Rotation, *pFromQuat, *pToQuat, fLerpProgress);

							if (pSeqBone->IsTranslateAnimated())
							{
								auto pFromTranslation = rKeyLibrary.GetT(pFromKey[2]);
								auto pToTranslation   = rKeyLibrary.GetT(pToKey[2]);

								rBoneCache.Position.Lerp(*pFromTranslation, *pToTranslation, fLerpProgress);
							}
							else
							{
								rBoneCache.Position = pBone->GetPosition();
							}
						}
						else
						{
							TVector4    position;
							TQuaternion rotation;

							QInterpFn(rotation, *pFromQuat, *pToQuat, fLerpProgress);

							if (pSeqBone->IsTranslateAnimated())
							{
								auto pFromTranslation = rKeyLibrary.GetT(pFromKey[2]);
								auto pToTranslation   = rKeyLibrary.GetT(pToKey[2]);

								position.Lerp3(*pFromTranslation, *pToTranslation, fLerpProgress);
							}

							fWeightTotalRatio += fWeightRatio;
							QInterpFn(rBoneCache.Rotation, rBoneCache.Rotation, rotation, fWeightRatio / fWeightTotalRatio);

							const TVector3* pLerpToVec = pSeqBone->IsTranslateAnimated() ? &position.AsVector3() : &pBone->GetPosition();
							rBoneCache.Position.Lerp(*pLerpToVec, fWeightRatio / fWeightTotalRatio);
						}
					}
				}

				if (!bBoneHasState)
				{
					rBoneCache.Rotation = pBone->GetRotation();
					rBoneCache.Position = pBone->GetPosition();
				}
			}

			for (auto it = m_OverlayAnimations.Begin(); it != m_OverlayAnimations.End(); it++)
			{
				if (it->GetWeight() != 0)
				{
					auto pSeq     = m_pSkeleton->GetSequence(it->GetSequence());
					auto pSeqBone = pSeq->GetBone(i);

					if (!pSeqBone->Is2() && pSeqBone->GetKeyCount() != 0)
					{
						TINT iCurrentKeyframePos = TINT((it->GetSeqTime() / pSeq->GetDuration()) * 65535);

						TUINT  iLerpFromIndex;
						TUINT  iLerpToIndex;
						TFLOAT fLerpProgress = pSeqBone->GetKeyPair(iCurrentKeyframePos, *it->GetBone(i), iLerpFromIndex, iLerpToIndex);

						auto pFromKey = pSeqBone->GetKey(iLerpFromIndex);
						auto pToKey   = pSeqBone->GetKey(iLerpToIndex);

						auto& rKeyLibrary = m_pSkeleton->GetKeyLibraryInstance();
						auto  pFromQuat   = rKeyLibrary.GetQ(pFromKey[1]);
						auto  pToQuat     = rKeyLibrary.GetQ(pToKey[1]);

						TVector4    position;
						TQuaternion rotation;

						QInterpFn(rotation, rBoneCache.Rotation, *pToQuat, fLerpProgress);
						QInterpFn(rBoneCache.Rotation, rBoneCache.Rotation, rotation, it->GetWeight());

						const TVector3* pLerpToVec;
						if (pSeqBone->IsTranslateAnimated())
						{
							auto pFromTranslation = rKeyLibrary.GetT(pFromKey[2]);
							auto pToTranslation   = rKeyLibrary.GetT(pToKey[2]);

							position.Lerp3(*pFromTranslation, *pToTranslation, fLerpProgress);
							pLerpToVec = &position.AsVector3();
						}
						else
						{
							pLerpToVec = &pBone->GetPosition();
						}

						rBoneCache.Position.Lerp(*pLerpToVec, it->GetWeight());
					}
				}
			}
		}

		for (TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++)
		{
			auto& rBoneCache = g_aBonesCaches[i];
			auto& rMatrix    = g_aForwardMatrices[i];

			auto pBone       = m_pSkeleton->GetBone(i);
			auto iParentBone = pBone->GetParentBone();

			if (iParentBone == -1)
			{
				// No parent bone
				rMatrix.SetFromQuaternion(rBoneCache.Rotation);
				rMatrix.SetTranslation(rBoneCache.Position);
			}
			else
			{
				// Has parent bone
				TMatrix44 boneTransform;
				boneTransform.SetFromQuaternion(rBoneCache.Rotation);
				boneTransform.SetTranslation(rBoneCache.Position);
				rMatrix.Multiply(g_aForwardMatrices[iParentBone], boneTransform);
			}

			m_pBones[i].Multiply(rMatrix, pBone->GetTransformInv());
		}
	}
}

// $deBlob: FUNCTION 0068db70
TMatrix44* TSkeletonInstance::GetBoneTransformCurrent(TINT a_iBone, TMatrix44& a_rMatrix)
{
	if (a_iBone < m_pSkeleton->GetAutoBoneCount())
	{
		auto pBone = m_pSkeleton->GetBone(a_iBone);
		a_rMatrix.Multiply(m_pBones[a_iBone], pBone->GetTransform());
		return &a_rMatrix;
	}

	a_rMatrix = m_pSkeleton->GetBone(a_iBone)->GetTransform();
	return &a_rMatrix;
}

// $deBlob: FUNCTION 0068c2c0
TAnimation* TSkeletonInstance::AddAnimationFull(TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed, TAnimation::Flags a_eAnimFlags)
{
	TAnimation* pAnimation  = IsAnimating(a_iSequenceIndex);
	TBOOL       bZeroWeight = TFALSE;

	if (pAnimation == TNULL)
	{
		bZeroWeight = TTRUE;
		pAnimation  = m_FreeAnimations.Begin();

		if (m_FreeAnimations.IsEmpty())
		{
			// No free animations left, so take an oldest
			// playing animation from the list of base animations
			TQList<TAnimation>* pList = &m_BaseAnimations;

			if (m_BaseAnimations.IsEmpty())
			{
				// No base animations are playing,
				// so use list of overlay animations
				pList = &m_OverlayAnimations;
			}

			pAnimation = m_OverlayAnimations.Begin();

			// Find animation with less weight than the one user wants to play
			for (auto it = pAnimation->Next(); it != pList->End(); it++)
			{
				if (it->GetWeight() <= pAnimation->GetWeight() && pAnimation->GetWeight() != it->GetWeight())
				{
					pAnimation = it;
				}
			}

			pAnimation->RemoveAnimation(0.0f);
		}

		pAnimation->Remove();
		auto pSequence = m_pSkeleton->GetSequence(a_iSequenceIndex);

		if (pSequence->IsOverlay())
		{
			m_OverlayAnimations.Push(pAnimation);
			m_iOverlayAnimationCount++;
		}
		else
		{
			m_BaseAnimations.Push(pAnimation);
			m_iBaseAnimationCount++;
		}
	}

	pAnimation->SetSequence(a_iSequenceIndex);

	pAnimation->SetSpeed(1.0f);
	pAnimation->SetUnk3(0);
	pAnimation->SetFlags(TAnimation::Flags_Active | a_eAnimFlags);

	if (bZeroWeight) pAnimation->SetWeight(0.0f);

	TMath::Clip(pAnimation->GetWeight(), 0.0f, 1.0f);

	TFLOAT fDestWeight = TMath::Min(TMath::Max(0.0f, a_fDestWeight), 1.0f);

	pAnimation->ResetTime();
	pAnimation->SetDestWeightExplicit(fDestWeight);
	pAnimation->SetBlendInSpeed(a_fBlendInSpeed);
	pAnimation->SetBlendOutSpeed(a_fBlendOutSpeed);

	if (a_fBlendInSpeed <= 0.0f)
	{
		pAnimation->SetWeight(fDestWeight);
		pAnimation->SetMode(TAnimation::MODE_UNK2);
	}
	else
	{
		pAnimation->SetMode(TAnimation::MODE_UNK1);
	}

	TUtil::MemClear(pAnimation->GetBones(), sizeof(TAnimationBone) * m_pSkeleton->GetAutoBoneCount());
	pAnimation->SetSkeletonInstance(this);

	SetSequenceMaxUnk3(TMath::Max(TMath::Max(GetSequenceMaxUnk3(), TAnimation::FindSequenceMaxUnk3(m_BaseAnimations)), TAnimation::FindSequenceMaxUnk3(m_OverlayAnimations)));

	return pAnimation;
}

// $deBlob: FUNCTION 0068c680
void TSkeletonInstance::RemoveAnimation(TAnimation* a_pAnimation, TFLOAT a_fBlendOutSpeed)
{
	TASSERT(TTRUE == a_pAnimation->IsActive());

	if (a_pAnimation)
	{
		if (a_fBlendOutSpeed <= 0.0f)
		{
			if (a_pAnimation->IsUpdateStateOnRemove())
			{
				a_pAnimation->SetUpdateStateOnRemove(TFALSE);
				a_pAnimation->SetWeight(1.0f);
				a_pAnimation->SetBlendOutSpeed(1.0f);
				a_pAnimation->SetMode(TAnimation::MODE_UNK3);
				m_iFlags |= 1;
				return;
			}

			auto pSeq = a_pAnimation->GetSequencePtr();

			if (TFALSE == pSeq->IsOverlay())
				m_iBaseAnimationCount--;
			else
				m_iOverlayAnimationCount--;

			a_pAnimation->SetFlags(TAnimation::Flags_None);
			a_pAnimation->Remove();
			m_FreeAnimations.PushFront(a_pAnimation);
		}
		else
		{
			a_pAnimation->SetBlendOutSpeed(a_fBlendOutSpeed);
			a_pAnimation->SetDestWeightExplicit(0.0f);
			a_pAnimation->SetMode(TAnimation::MODE_UNK3);
		}

		SetSequenceMaxUnk3(TMath::Max(TMath::Max(GetSequenceMaxUnk3(), TAnimation::FindSequenceMaxUnk3(m_BaseAnimations)), TAnimation::FindSequenceMaxUnk3(m_OverlayAnimations)));
	}
}

// $deBlob: FUNCTION 0068cb10
void TSkeletonInstance::SetStateFromBasePose()
{
	for (TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++)
	{
		m_pSkeleton->GetBone(i);
		m_pBones[i].Identity();
	}
}

TAnimation* TSkeletonInstance::IsAnimating(TINT16 a_iSequenceIndex)
{
	TASSERT(a_iSequenceIndex < m_pSkeleton->GetSequenceCount());

	if (m_pSkeleton->GetSequenceCount() <= a_iSequenceIndex) return TNULL;

	for (auto it = m_BaseAnimations.Begin(); it != m_BaseAnimations.End(); it++)
	{
		if (it->GetSequence() == a_iSequenceIndex) return it;
	}

	for (auto it = m_OverlayAnimations.Begin(); it != m_OverlayAnimations.End(); it++)
	{
		if (it->GetSequence() == a_iSequenceIndex) return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0068e910
TFLOAT TSkeletonSequenceBone::GetKeyPair(TINT a_iCurrentAnimTime, TUINT& a_rCurrentKeyIndex, TUINT& a_rLerpFromIndex, TUINT& a_rLerpToIndex)
{
	auto pFirstKeyTime = *GetKey(0);

	if (a_iCurrentAnimTime < pFirstKeyTime || a_iCurrentAnimTime == pFirstKeyTime)
	{
		// Animation haven't reached it's first frame
		a_rCurrentKeyIndex = 0;
		a_rLerpFromIndex   = 0;
		a_rLerpToIndex     = 0;
		return 0.0f;
	}

	auto iLastKeyIndex = m_iNumKeys - 1;
	auto pLastKeyTime  = *GetKey(iLastKeyIndex);

	if (pLastKeyTime <= a_iCurrentAnimTime)
	{
		// Animation is over
		a_rCurrentKeyIndex = iLastKeyIndex;
		a_rLerpFromIndex   = iLastKeyIndex;
		a_rLerpToIndex     = iLastKeyIndex;
		return 0.0f;
	}

	auto pCurrentKeyTime = *GetKey(a_rCurrentKeyIndex);

	if (pCurrentKeyTime < a_iCurrentAnimTime)
	{
		// Current key is currently lerping
		auto iNextIndex   = a_rCurrentKeyIndex + 1;
		auto iNextKeyTime = *GetKey(iNextIndex);

		while (iNextKeyTime <= a_iCurrentAnimTime)
		{
			// Skip keys that are already over
			a_rCurrentKeyIndex = iNextIndex++;
			iNextKeyTime       = *GetKey(iNextIndex);
		}

		a_rLerpFromIndex = a_rCurrentKeyIndex;
		a_rLerpToIndex   = iNextIndex;
	}
	else
	{
		if (pCurrentKeyTime == a_iCurrentAnimTime)
		{
			// Current time is right at the end of keys transition
			a_rLerpFromIndex = a_rCurrentKeyIndex;
			a_rLerpToIndex   = a_rCurrentKeyIndex;
			return 0.0f;
		}

		// The animation is playing backwards?
		auto iPrevIndex   = a_rCurrentKeyIndex - 1;
		auto iPrevKeyTime = *GetKey(iPrevIndex);

		while (a_iCurrentAnimTime < iPrevKeyTime || a_iCurrentAnimTime == iPrevKeyTime)
		{
			a_rCurrentKeyIndex = iPrevIndex--;
			iPrevKeyTime       = *GetKey(iPrevIndex);
		}

		a_rLerpFromIndex = iPrevIndex;
		a_rLerpToIndex   = a_rCurrentKeyIndex;
	}

	auto iLerpFromTime = *GetKey(a_rLerpFromIndex);
	auto iLerpToTime   = *GetKey(a_rLerpToIndex);

	return ((a_iCurrentAnimTime - iLerpFromTime) * (1.0f / 65535)) / ((iLerpToTime - iLerpFromTime) * (1.0f / 65535));
}

TOSHI_NAMESPACE_END
