#include "ToshiPCH.h"
#include "TSkeleton.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TMatrix44 TSkeletonInstance::g_aForwardMatrices[ TANIMATION_MAXBONES ];
// Dynamic initializer (a loop over the array) and the implicit BoneState() it calls (only caller)
// $deBlob: FUNCTION 0053eb00
// $deBlob: FUNCTION 0068f6a0
TSkeletonInstance::BoneState TSkeletonInstance::g_aBoneStates[ TANIMATION_MAXBONES ];


// $deBlob: FUNCTION 0068f4d0
TSkeleton::TSkeleton()
{
	m_iBoneCount         = 0;
	m_iManualBoneCount   = 0;
	m_iSequenceCount     = 0;
	m_iAnimationMaxCount = 0;
	m_iInstanceCount     = 0;
	m_pBones             = TNULL;
	m_SkeletonSequences  = TNULL;
	m_fnQuatLerp         = TNULL;
}

// $deBlob: FUNCTION 0068f510
TSkeleton::~TSkeleton()
{
	TASSERT( TNULL == m_pBones );
	TASSERT( TNULL == m_SkeletonSequences );

	// m_KeyLibraryInstance's destructor releases the library
}

// $deBlob: FUNCTION 0068f4a0
TINT16 TSkeleton::SetAnimationMaxCount( TINT16 a_iAnimationMaxCount )
{
	TINT16 iOldCount     = m_iAnimationMaxCount;
	m_iAnimationMaxCount = TMath::Min( a_iAnimationMaxCount, m_iSequenceCount );

	return iOldCount;
}

// $deBlob: FUNCTION 0068ed60
// $Barnyard: FUNCTION 006ca760
TSkeletonInstance* TSkeleton::CreateInstance( TBOOL a_bSetBasePose )
{
	// Set quaternion interpolation method if it's not already set
	if ( TINT( m_eQuatLerpType ) < 3 )
		SetQInterpFn( m_eQuatLerpType );

	auto  iAutoBoneCount = GetAutoBoneCount();
	TSIZE iBonesSize     = TAlignNumUp( iAutoBoneCount * sizeof( TSkeletonInstanceBone ) );
	TSIZE iAnimationSize = TAlignNumUp( iAutoBoneCount * sizeof( TAnimationBone ) + sizeof( TAnimation ) );
	TSIZE iInstanceSize  = iBonesSize + iAnimationSize * GetAnimationMaxCount() + sizeof( TSkeletonInstance );

	TMemoryHeap*       pHeap     = GetTAnimMemoryHeap();
	TSkeletonInstance* pInstance = TSTATICCAST( TSkeletonInstance, ( pHeap != TNULL ) ? TMemoryHeap_Malloc( pHeap, iInstanceSize ) : TMalloc( iInstanceSize ) );

	if ( pInstance == TNULL )
		return TNULL;

	new ( pInstance ) TSkeletonInstance();
	pInstance->m_iSize                  = iInstanceSize;
	pInstance->m_iBaseAnimationCount    = 0;
	pInstance->m_iOverlayAnimationCount = 0;
	pInstance->m_eFlags                 = TSkeletonInstance::FLAGS_NONE;
	pInstance->m_pSkeleton              = this;
	pInstance->m_pBones                 = TREINTERPRETCAST( TSkeletonInstanceBone*, pInstance + 1 );
	pInstance->m_pAnimations            = TREINTERPRETCAST( TAnimation*, TREINTERPRETCAST( TUINTPTR, pInstance->m_pBones ) + iBonesSize );
	pInstance->m_fTotalWeight           = 0.0f;
	pInstance->m_iLastUpdateTimeFrame   = 0;
	pInstance->m_iLastUpdateStateFrame  = 0;
	pInstance->m_iMaxBoundingSphereIndex = -1;

	for ( TINT i = 0; i < GetAnimationMaxCount(); i++ )
	{
		TAnimation* pAnimation = TREINTERPRETCAST(
		    TAnimation*,
		    TREINTERPRETCAST( TUINTPTR, pInstance->m_pAnimations ) + i * iAnimationSize
		);

		new ( pAnimation ) TAnimation();
		pInstance->m_FreeAnimations.PushFront( pAnimation );
	}

	if ( a_bSetBasePose == TTRUE )
		pInstance->SetStateFromBasePose();

	m_iInstanceCount++;
	return pInstance;
}

// $deBlob: FUNCTION 0068f480
void TSkeleton::SetQInterpFn( QUATINTERP a_eQuatInterp )
{
	if ( a_eQuatInterp == QUATINTERP_Slerp )
		m_fnQuatLerp = TQuaternion::Slerp;
	else
		m_fnQuatLerp = TQuaternion::Nlerp;
}

// $deBlob: FUNCTION 0068ee90
// $Barnyard: FUNCTION 006c9d10
TINT16 TSkeleton::GetBoneID( const TCHAR* a_szBoneName, TINT a_iLength )
{
	for ( TINT i = 0; i < m_iBoneCount; i++ )
	{
		if ( T2String8::CompareNoCase( m_pBones[ i ].GetName(), a_szBoneName, a_iLength ) == 0 )
			return i;
	}

	return TBONE_INVALID;
}

// $deBlob: FUNCTION 0068eee0
// $Barnyard: FUNCTION 006c9d80
TINT16 TSkeleton::GetSequenceID( const TCHAR* a_szSequenceName, TINT a_iLength )
{
	for ( TINT i = 0; i < m_iSequenceCount; i++ )
	{
		if ( T2String8::CompareNoCase( m_SkeletonSequences[ i ].GetName(), a_szSequenceName, a_iLength ) == 0 )
			return i;
	}

	return TSEQ_INVALID;
}

// $deBlob: FUNCTION 0068ef30
void TSkeleton::DebugDump( TBOOL a_bDumpKeys )
{
	// NOTE: the output is compiled out in retail, only the accessors (and their asserts) are left
	for ( TINT i = 0; i < m_iBoneCount; i++ )
	{
		TSkeletonBone* pBone = GetBone( i );

		if ( pBone->GetParentBone() != TBONE_INVALID )
			GetBone( pBone->GetParentBone() );
	}

	TKeyframeLibraryInstance* pKeyLib = &m_KeyLibraryInstance;

	for ( TINT i = 0; i < m_iSequenceCount; i++ )
	{
		TSkeletonSequence* pSeq = GetSequence( i );

		for ( TINT j = 0; j < m_iBoneCount; j++ )
		{
			GetBone( j );
			TSkeletonSequenceBone* pSeqBone = pSeq->GetBone( j );

			if ( pKeyLib != TNULL && a_bDumpKeys == TTRUE )
			{
				for ( TINT k = 0; k < pSeqBone->GetKeyCount(); k++ )
				{
					TASSERT( ( k >= 0 ) && ( k <= TMath::TUINT16_MAX ) );
					TUINT16* pKey = pSeqBone->GetKey( TUINT16( k ) );

					pKeyLib->GetQ( pKey[ 1 ] );

					if ( pSeqBone->IsTranslateAnimated() )
						pKeyLib->GetT( pKey[ 2 ] );
				}
			}
		}
	}
}

// $deBlob: FUNCTION 0068cf60
// $Barnyard: FUNCTION 006cb420
void TSkeletonInstance::UpdateTime( TFLOAT a_fDeltaTime )
{
	// Advance the animations at most once per frame (several systems call this for the same skeleton)
	if ( ( !m_BaseAnimations.IsEmpty() || !m_OverlayAnimations.IsEmpty() ) && m_iLastUpdateTimeFrame != TSystemManager::GetSingleton()->GetFrameCount() )
	{
		m_iLastUpdateTimeFrame = TSystemManager::GetSingleton()->GetFrameCount();
		m_fTotalWeight         = 0.0f;

		// Update base animations
		for ( auto pAnim = m_BaseAnimations.Begin(); pAnim != m_BaseAnimations.End(); )
		{
			// The animation can remove itself
			auto pCurrent = pAnim++;

			if ( pCurrent->UpdateTime( a_fDeltaTime ) == TTRUE )
			{
				// NOTE: retail looks the sequence up and doesn't use it
				m_pSkeleton->GetSequence( pCurrent->GetSequence() );
				m_fTotalWeight += pCurrent->GetWeight();
			}
		}

		// Update overlay animations
		for ( auto pAnim = m_OverlayAnimations.Begin(); pAnim != m_OverlayAnimations.End(); )
		{
			auto pCurrent = pAnim++;
			pCurrent->UpdateTime( a_fDeltaTime );
		}

		if ( HASANYFLAG( m_eFlags, FLAGS_UPDATE_STATE ) )
		{
			m_eFlags &= ~FLAGS_UPDATE_STATE;
			UpdateState( TFALSE );
		}
	}
}

// $deBlob: FUNCTION 0068db60
// $Barnyard: FUNCTION 006c9e00
void TSkeletonInstance::UpdateStateFrame()
{
	m_eFlags &= ~FLAGS_UPDATE_STATE;
	m_iLastUpdateStateFrame = TSystemManager::GetSingleton()->GetFrameCount();
}

// $deBlob: FUNCTION 0068d0e0
// $Barnyard: FUNCTION 006ca960
void TSkeletonInstance::UpdateState( TBOOL a_bForceUpdate )
{
	if ( !a_bForceUpdate && m_iLastUpdateStateFrame == TSystemManager::GetSingleton()->GetFrameCount() )
		return;

	TKeyframeLibraryInstance& rKeyLibrary = m_pSkeleton->GetKeyLibraryInstance();

	if ( rKeyLibrary.GetLibrary() == TNULL )
		return;

	m_iLastUpdateStateFrame = TSystemManager::GetSingleton()->GetFrameCount();

	TFLOAT fOneOverTotalWeight = 1.0f;

	if ( m_fTotalWeight > 1.0f )
		fOneOverTotalWeight = 1.0f / m_fTotalWeight;

	const auto QInterpFn = m_pSkeleton->GetQInterpFn();

	TASSERT( m_pSkeleton->GetAutoBoneCount() < TANIMATION_MAXBONES );

	// Blend the animations into the bone states
	for ( TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++ )
	{
		TASSERT( i < TANIMATION_MAXBONES );

		BoneState&     rBoneState = g_aBoneStates[ i ];
		TSkeletonBone* pBone      = m_pSkeleton->GetBone( i );

		TBOOL  bHasState         = TFALSE;
		TFLOAT fWeightTotalRatio = 0.0f;

		T2_FOREACH( m_BaseAnimations, it )
		{
			TSkeletonSequence*     pSeq     = m_pSkeleton->GetSequence( it->GetSequence() );
			TSkeletonSequenceBone* pSeqBone = pSeq->GetBone( i );

			TINT iCurrentKeyframePos = TINT( ( it->GetSeqTime() / pSeq->GetDuration() ) * 65535.0f );

			TUINT16 iLerpFromIndex;
			TUINT16 iLerpToIndex;
			TFLOAT  fLerpProgress = pSeqBone->GetKeyPair( TUINT16( iCurrentKeyframePos ), *it->GetBone( i ), iLerpFromIndex, iLerpToIndex );

			TFLOAT fWeightRatio = it->GetWeight() * fOneOverTotalWeight;

			if ( fWeightRatio > 0.0f && pSeqBone->GetKeyCount() != 0 )
			{
				TUINT16* pFromKey = pSeqBone->GetKey( iLerpFromIndex );
				TUINT16* pToKey   = pSeqBone->GetKey( iLerpToIndex );

				TAnimQuaternion* pFromQuat = rKeyLibrary.GetQ( pFromKey[ 1 ] );
				TAnimQuaternion* pToQuat   = rKeyLibrary.GetQ( pToKey[ 1 ] );

				if ( it == m_BaseAnimations.Begin() )
				{
					// The first animation sets the state
					fWeightTotalRatio = fWeightRatio;
					bHasState         = TTRUE;
					QInterpFn( rBoneState.m_Rotation, *pFromQuat, *pToQuat, fLerpProgress );

					if ( pSeqBone->IsTranslateAnimated() )
						rBoneState.m_Position.Lerp( *rKeyLibrary.GetT( pFromKey[ 2 ] ), *rKeyLibrary.GetT( pToKey[ 2 ] ), fLerpProgress );
					else
						rBoneState.m_Position = pBone->GetPosition();
				}
				else
				{
					TVector4    position;
					TQuaternion rotation;

					QInterpFn( rotation, *pFromQuat, *pToQuat, fLerpProgress );

					// NOTE: retail passes the (12 byte) translations to TVector4::Lerp3 as they are, w isn't used
					if ( pSeqBone->IsTranslateAnimated() )
						position.Lerp3( *rKeyLibrary.GetT( pFromKey[ 2 ] ), *rKeyLibrary.GetT( pToKey[ 2 ] ), fLerpProgress );

					TFLOAT fRatio = fWeightRatio / ( fWeightRatio + fWeightTotalRatio );
					fWeightTotalRatio += fWeightRatio;

					QInterpFn( rBoneState.m_Rotation, rBoneState.m_Rotation, rotation, fRatio );

					const TVector3* pLerpToVec = pSeqBone->IsTranslateAnimated() ? &position.AsVector3() : &pBone->GetPosition();
					rBoneState.m_Position.Lerp( *pLerpToVec, fRatio );
				}
			}
		}

		// Bones that no base animation affects take the base pose
		if ( !bHasState )
		{
			rBoneState.m_Rotation = pBone->GetRotation();
			rBoneState.m_Position = pBone->GetPosition();
		}

		T2_FOREACH( m_OverlayAnimations, it )
		{
			if ( it->GetWeight() != 0.0f )
			{
				TSkeletonSequence*     pSeq     = m_pSkeleton->GetSequence( it->GetSequence() );
				TSkeletonSequenceBone* pSeqBone = pSeq->GetBone( i );

				if ( !pSeqBone->IsOverlayAnimated() && pSeqBone->GetKeyCount() != 0 )
				{
					TINT iCurrentKeyframePos = TINT( ( it->GetSeqTime() / pSeq->GetDuration() ) * 65535.0f );

					TUINT16 iLerpFromIndex;
					TUINT16 iLerpToIndex;
					TFLOAT  fLerpProgress = pSeqBone->GetKeyPair( TUINT16( iCurrentKeyframePos ), *it->GetBone( i ), iLerpFromIndex, iLerpToIndex );

					TUINT16* pFromKey = pSeqBone->GetKey( iLerpFromIndex );
					TUINT16* pToKey   = pSeqBone->GetKey( iLerpToIndex );

					TAnimQuaternion* pFromQuat = rKeyLibrary.GetQ( pFromKey[ 1 ] );
					TAnimQuaternion* pToQuat   = rKeyLibrary.GetQ( pToKey[ 1 ] );

					TVector4    position;
					TQuaternion rotation;

					QInterpFn( rotation, *pFromQuat, *pToQuat, fLerpProgress );
					QInterpFn( rBoneState.m_Rotation, rBoneState.m_Rotation, rotation, it->GetWeight() );

					const TVector3* pLerpToVec;

					if ( pSeqBone->IsTranslateAnimated() )
					{
						position.Lerp3( *rKeyLibrary.GetT( pFromKey[ 2 ] ), *rKeyLibrary.GetT( pToKey[ 2 ] ), fLerpProgress );
						pLerpToVec = &position.AsVector3();
					}
					else
					{
						pLerpToVec = &pBone->GetPosition();
					}

					rBoneState.m_Position.Lerp( *pLerpToVec, it->GetWeight() );
				}
			}
		}
	}

	// Build the matrices from the bone states
	for ( TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++ )
	{
		BoneState&     rBoneState  = g_aBoneStates[ i ];
		TMatrix44&     rMatrix     = g_aForwardMatrices[ i ];
		TSkeletonBone* pBone       = m_pSkeleton->GetBone( i );
		TINT           iParentBone = pBone->GetParentBone();

		if ( iParentBone == TBONE_INVALID )
		{
			// No parent bone
			rMatrix.SetFromQuaternion( rBoneState.m_Rotation );
			rMatrix.GetTranslation3() = rBoneState.m_Position;
		}
		else
		{
			// Has parent bone
			TMatrix44 boneTransform;
			boneTransform.SetFromQuaternion( rBoneState.m_Rotation );
			boneTransform.GetTranslation3() = rBoneState.m_Position;
			rMatrix.Multiply( g_aForwardMatrices[ iParentBone ], boneTransform );
		}

		m_pBones[ i ].m_Transform.Multiply( rMatrix, pBone->GetTransformInv() );
	}
}

// $deBlob: FUNCTION 0068db70
// $Barnyard: FUNCTION 006c9ec0
TMatrix44& TSkeletonInstance::GetBoneTransformCurrent( TINT a_iBone, TMatrix44& a_rOutTransform )
{
	if ( a_iBone < m_pSkeleton->GetAutoBoneCount() )
	{
		auto pBone = m_pSkeleton->GetBone( a_iBone );
		a_rOutTransform.Multiply( m_pBones[ a_iBone ].m_Transform, pBone->GetTransform() );

		return a_rOutTransform;
	}

	a_rOutTransform = m_pSkeleton->GetBone( a_iBone )->GetTransform();

	return a_rOutTransform;
}

// $deBlob: FUNCTION 0068dc60
// $Barnyard: FUNCTION 006c9f20
const TMatrix44& TSkeletonInstance::GetBoneTransformCurrent( TINT a_iBone )
{
	static TMatrix44 s_mDestMatrix;

	return GetBoneTransformCurrent( a_iBone, s_mDestMatrix );
}

// $deBlob: FUNCTION 0068c680
// $Barnyard: FUNCTION 006ca860
void TSkeletonInstance::RemoveAnimation( TAnimation* a_pAnimation, TFLOAT a_fBlendOutSpeed )
{
	// NOTE: retail asserts before the null check
	TASSERT( TTRUE == a_pAnimation->IsActive() );

	if ( !a_pAnimation )
		return;

	if ( a_fBlendOutSpeed <= 0.0f )
	{
		if ( a_pAnimation->m_eFlags & TAnimation::Flags_UpdateStateOnRemove )
		{
			// Keep the animation for one more state update at full weight, then blend it out
			a_pAnimation->m_fWeight = 1.0f;
			a_pAnimation->m_eFlags &= ~TAnimation::Flags_UpdateStateOnRemove;
			a_pAnimation->m_fBlendOutSpeed = 1.0f;
			a_pAnimation->m_eState         = TAnimation::STATE_BLENDING_OUT;
			m_eFlags |= FLAGS_UPDATE_STATE;
			return;
		}

		if ( a_pAnimation->GetSequencePtr()->IsOverlay() )
			m_iOverlayAnimationCount -= 1;
		else
			m_iBaseAnimationCount -= 1;

		a_pAnimation->m_eFlags = TAnimation::Flags_None;

		// Unlink the animation and add it to the list of free animations
		a_pAnimation->Remove();
		m_FreeAnimations.PushFront( a_pAnimation );
	}
	else
	{
		a_pAnimation->m_fBlendOutSpeed = a_fBlendOutSpeed;
		a_pAnimation->m_fDestWeight    = 0.0f;
		a_pAnimation->m_eState         = TAnimation::STATE_BLENDING_OUT;
	}

	UpdateMaxBoundingSphereIndex();
}

// $deBlob: FUNCTION 0068cb10
// $Barnyard: FUNCTION 006c9e20
void TSkeletonInstance::SetStateFromBasePose()
{
	// NOTE: de Blob's instance bones are only matrices, Barnyard also copies the base pose rotation and position
	for ( TINT i = 0; i < m_pSkeleton->GetAutoBoneCount(); i++ )
	{
		m_pSkeleton->GetBone( i );
		m_pBones[ i ].m_Transform.Identity();
	}
}

// $deBlob: FUNCTION 0068c2b0
// $Barnyard: FUNCTION 006c9df0
void TSkeletonInstance::Delete()
{
	m_pSkeleton->m_iInstanceCount--;
	TFree( this );
}

// $deBlob: FUNCTION 0068c790
// $Barnyard: FUNCTION 006cb3a0
void TSkeletonInstance::RemoveAllAnimations( TFLOAT a_fBlendOutSpeed )
{
	if ( a_fBlendOutSpeed <= 0.0f )
	{
		while ( TAnimation* pAnimation = m_BaseAnimations.IsEmpty() ? TNULL : &*m_BaseAnimations.Begin() )
			RemoveAnimation( pAnimation, 0.0f );

		while ( TAnimation* pAnimation = m_OverlayAnimations.IsEmpty() ? TNULL : &*m_OverlayAnimations.Begin() )
			RemoveAnimation( pAnimation, 0.0f );
	}
	else
	{
		for ( auto it = m_BaseAnimations.Begin(); it != m_BaseAnimations.End(); )
		{
			auto pAnimation = it++;
			RemoveAnimation( pAnimation, a_fBlendOutSpeed );
		}

		for ( auto it = m_OverlayAnimations.Begin(); it != m_OverlayAnimations.End(); )
		{
			auto pAnimation = it++;
			RemoveAnimation( pAnimation, a_fBlendOutSpeed );
		}
	}

	m_iMaxBoundingSphereIndex = -1;
}

// $Barnyard: FUNCTION 006cb3f0
void TSkeletonInstance::RemoveAllBaseAnimations()
{
	while ( !m_BaseAnimations.IsEmpty() )
		RemoveAnimation( m_BaseAnimations.Begin(), 0.0f );
}

// $deBlob: FUNCTION 0068c8d0
void TSkeletonInstance::MoveAnimationToFront( TAnimation* a_pAnimation )
{
	TASSERT( a_pAnimation->IsLinked() );
	a_pAnimation->Remove();

	if ( a_pAnimation->GetSequencePtr()->IsOverlay() )
		m_OverlayAnimations.PushFront( a_pAnimation );
	else
		m_BaseAnimations.PushFront( a_pAnimation );
}

// $deBlob: FUNCTION 0068c9d0
void TSkeletonInstance::MoveAnimationToBack( TAnimation* a_pAnimation )
{
	TASSERT( a_pAnimation->IsLinked() );
	a_pAnimation->Remove();

	if ( a_pAnimation->GetSequencePtr()->IsOverlay() )
		m_OverlayAnimations.Push( a_pAnimation );
	else
		m_BaseAnimations.Push( a_pAnimation );
}

// NOTE: de Blob's TSkeletonInstance::Delete frees the memory without running a destructor
TSkeletonInstance::~TSkeletonInstance()
{
}

// $deBlob: FUNCTION 0068cad0
// $Barnyard: FUNCTION 006ca920
TAnimation* TSkeletonInstance::IsAnimating( TINT16 a_iSeqId )
{
	T2_FOREACH( m_BaseAnimations, it )
	{
		if ( it->GetSequence() == a_iSeqId )
			return it;
	}

	T2_FOREACH( m_OverlayAnimations, it )
	{
		if ( it->GetSequence() == a_iSeqId )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0068c2c0
// $Barnyard: FUNCTION 006cb190
TAnimation* TSkeletonInstance::AddAnimationFull( TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed, TUINT32 a_uiFlags )
{
	TSkeleton* pSkeleton = m_pSkeleton;
	TASSERT( a_iSequenceIndex < pSkeleton->GetSequenceCount() );

	if ( a_iSequenceIndex >= pSkeleton->GetSequenceCount() )
		return TNULL;

	// Is the sequence already playing?
	TAnimation* pAnimation = IsAnimating( a_iSequenceIndex );
	TBOOL       bIsPlaying = pAnimation != TNULL;

	if ( !bIsPlaying )
	{
		pAnimation = GetFreeAnimation();

		// If the free list is empty, replace the animation with the least weight
		if ( pAnimation == TNULL )
		{
			TQList<TAnimation>* pIterateList = &m_BaseAnimations;

			// If no base animations are currently playing, get one from the overlay animations list
			if ( m_BaseAnimations.IsEmpty() )
				pIterateList = &m_OverlayAnimations;

			auto it    = pIterateList->Begin();
			pAnimation = it++;

			for ( ; it != pIterateList->End(); it++ )
			{
				if ( pAnimation->GetWeight() > it->GetWeight() )
					pAnimation = it;
			}

			// Instantly remove the animation
			pAnimation->RemoveAnimation( 0.0f );
		}

		// Remove animation from any list
		pAnimation->Remove();

		if ( pSkeleton->GetSequence( a_iSequenceIndex )->IsOverlay() )
		{
			// Overlay Animation
			m_OverlayAnimations.Push( pAnimation );
			m_iOverlayAnimationCount++;
		}
		else
		{
			// Base Animation
			m_BaseAnimations.Push( pAnimation );
			m_iBaseAnimationCount++;
		}
	}

	pAnimation->m_iSeqID = a_iSequenceIndex;

	TFLOAT fDestWeight = TMath::Min( TMath::Max( a_fDestWeight, 0.0f ), 1.0f );

	TASSERT( a_uiFlags <= TMath::TUINT16_MAX );
	pAnimation->m_fSpeed = 1.0f;
	pAnimation->m_eFlags = TAnimation::Flags( a_uiFlags | TAnimation::Flags_Active );
	pAnimation->m_iUnk3  = 0;

	if ( !bIsPlaying )
		pAnimation->m_fWeight = 0.0f;

	TMath::Clip( pAnimation->m_fWeight, 0.0f, 1.0f );

	pAnimation->m_fTotalTime     = 0.0f;
	pAnimation->m_fSeqTime       = 0.0f;
	pAnimation->m_fDestWeight    = fDestWeight;
	pAnimation->m_fBlendInSpeed  = a_fBlendInSpeed;
	pAnimation->m_fBlendOutSpeed = a_fBlendOutSpeed;

	if ( a_fBlendInSpeed <= 0.0f )
	{
		pAnimation->m_fWeight = fDestWeight;
		pAnimation->m_eState  = TAnimation::STATE_PLAYING;
	}
	else
	{
		pAnimation->m_eState = TAnimation::STATE_BLENDING_IN;
	}

	// Reset the current keys
	TUtil::MemSet( pAnimation->GetBones(), 0, pSkeleton->GetAutoBoneCount() * sizeof( TAnimationBone ) );

	pAnimation->m_pSkeletonInstance = this;
	UpdateMaxBoundingSphereIndex();

	return pAnimation;
}

// $deBlob: FUNCTION 0068c600
// $Barnyard: FUNCTION 006cb4d0
TAnimation* TSkeletonInstance::AddAnimation( TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed )
{
	return AddAnimationFull( a_iSequenceIndex, a_fDestWeight, a_fBlendInSpeed, 0.0f, TAnimation::Flags_None );
}

// $deBlob: FUNCTION 0068c640
TAnimation* TSkeletonInstance::AddManagedAnimation( TINT16 a_iSequenceIndex, TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed, TFLOAT a_fBlendOutSpeed )
{
	return AddAnimationFull( a_iSequenceIndex, a_fDestWeight, a_fBlendInSpeed, a_fBlendOutSpeed, TAnimation::Flags_Managed );
}

// $deBlob: FUNCTION 0068cb90
void TSkeletonInstance::CopyAnimations( TSkeletonInstance* a_pSrc )
{
#ifdef DEBLOB_COMMUNITY_PATCH
	TQList<TAnimation>& rBaseList = m_BaseAnimations;
#else
	// NOTE: retail bug: the loop that frees the base animations moves the free list's last animation to its front
	// instead, so it never ends while a base animation plays (CopyAnimations isn't called in retail)
	TQList<TAnimation>& rBaseList = m_FreeAnimations;
#endif

	// Free the playing animations
	while ( !m_BaseAnimations.IsEmpty() )
	{
		TAnimation* pAnimation = rBaseList.IsEmpty() ? TNULL : rBaseList.End()->Prev();
		if ( pAnimation ) pAnimation->Remove();
		m_FreeAnimations.PushFront( pAnimation );
	}

	while ( !m_OverlayAnimations.IsEmpty() )
	{
		TAnimation* pAnimation = m_OverlayAnimations.End()->Prev();
		pAnimation->Remove();
		m_FreeAnimations.PushFront( pAnimation );
	}

	m_eFlags                 = a_pSrc->m_eFlags;
	m_iBaseAnimationCount    = 0;
	m_iOverlayAnimationCount = 0;
	m_fTotalWeight           = a_pSrc->m_fTotalWeight;
	m_iLastUpdateTimeFrame   = a_pSrc->m_iLastUpdateTimeFrame;
	m_iLastUpdateStateFrame  = a_pSrc->m_iLastUpdateStateFrame;
	m_iMaxBoundingSphereIndex = a_pSrc->m_iMaxBoundingSphereIndex;

	TASSERT( TFALSE == m_FreeAnimations.IsEmpty() );

	T2_FOREACH( a_pSrc->m_BaseAnimations, it )
	{
		TAnimation* pDst = GetFreeAnimation();
		TASSERT( TNULL != pDst, "Skeleton instances must share the same keyframe library or animations" );
		TASSERT( TFALSE == ( pDst == m_FreeAnimations.End() ) );

		CopyAnimation( pDst, it );
		pDst->m_pSkeletonInstance = this;
		pDst->Remove();
		m_BaseAnimations.Push( pDst );
		m_iBaseAnimationCount++;
	}

	T2_FOREACH( a_pSrc->m_OverlayAnimations, it )
	{
		TAnimation* pDst = GetFreeAnimation();
		TASSERT( TNULL != pDst, "Skeleton instances must share the same keyframe library or animations" );
		TASSERT( TFALSE == ( pDst == m_FreeAnimations.End() ) );

		CopyAnimation( pDst, it );
		pDst->m_pSkeletonInstance = this;
		pDst->Remove();
		m_OverlayAnimations.Push( pDst );
		m_iOverlayAnimationCount++;
	}
}

// $deBlob: FUNCTION 0068dcc0
void __stdcall TSkeletonInstance::CopyAnimation( TAnimation* a_pDst, TAnimation* a_pSrc )
{
	a_pDst->m_pSkeletonInstance = a_pSrc->m_pSkeletonInstance;
	a_pDst->m_iSeqID            = a_pSrc->m_iSeqID;
	a_pDst->m_eFlags            = a_pSrc->m_eFlags;
	a_pDst->m_iUnk3             = a_pSrc->m_iUnk3;
	a_pDst->m_eState            = a_pSrc->m_eState;
	a_pDst->m_fSpeed            = a_pSrc->m_fSpeed;
	a_pDst->m_fWeight           = a_pSrc->m_fWeight;
	a_pDst->m_fDestWeight       = a_pSrc->m_fDestWeight;
	a_pDst->m_fTotalTime        = a_pSrc->m_fTotalTime;
	a_pDst->m_fSeqTime          = a_pSrc->m_fSeqTime;
	a_pDst->m_fBlendInSpeed     = a_pSrc->m_fBlendInSpeed;
	a_pDst->m_fBlendOutSpeed    = a_pSrc->m_fBlendOutSpeed;

	TUtil::MemCopy( a_pDst->GetBones(), a_pSrc->GetBones(), a_pSrc->m_pSkeletonInstance->GetSkeleton()->GetAutoBoneCount() * sizeof( TAnimationBone ) );
}

const TSkeletonInstanceBone& TSkeletonInstance::GetBone( TINT a_uiIndex )
{
	TASSERT( a_uiIndex < m_pSkeleton->GetAutoBoneCount() );
	return m_pBones[ a_uiIndex ];
}

TSkeletonInstance::TSkeletonInstance()
{
}

// $deBlob: FUNCTION 0068e910
// $Barnyard: FUNCTION 006c9f70
TFLOAT TSkeletonSequenceBone::GetKeyPair( TUINT16 a_uiCurrentAnimTime, TUINT16& a_rCurrentKeyIndex, TUINT16& a_rLerpFromIndex, TUINT16& a_rLerpToIndex )
{
	if ( a_uiCurrentAnimTime <= *GetKey( 0 ) )
	{
		// Animation haven't reached it's first frame
		a_rCurrentKeyIndex = 0;
		a_rLerpFromIndex   = 0;
		a_rLerpToIndex     = 0;
		return 0.0f;
	}

	TUINT16 uiLastKeyIndex = m_iNumKeys - 1;

	if ( a_uiCurrentAnimTime >= *GetKey( uiLastKeyIndex ) )
	{
		// Animation is over
		a_rCurrentKeyIndex = uiLastKeyIndex;
		a_rLerpFromIndex   = uiLastKeyIndex;
		a_rLerpToIndex     = a_rCurrentKeyIndex;
		return 0.0f;
	}

	TUINT16 uiCurrentKeyTime = *GetKey( a_rCurrentKeyIndex );

	if ( a_uiCurrentAnimTime > uiCurrentKeyTime )
	{
		// Skip the keys that are already over
		TUINT16 uiNextIndex = a_rCurrentKeyIndex + 1;

		while ( a_uiCurrentAnimTime >= *GetKey( uiNextIndex ) )
			a_rCurrentKeyIndex = uiNextIndex++;

		a_rLerpFromIndex = a_rCurrentKeyIndex;
		a_rLerpToIndex   = uiNextIndex;
	}
	else if ( a_uiCurrentAnimTime < uiCurrentKeyTime )
	{
		// The animation is playing backwards
		TUINT16 uiPrevIndex = a_rCurrentKeyIndex - 1;

		while ( a_uiCurrentAnimTime <= *GetKey( uiPrevIndex ) )
			a_rCurrentKeyIndex = uiPrevIndex--;

		a_rLerpFromIndex = uiPrevIndex;
		a_rLerpToIndex   = a_rCurrentKeyIndex;
	}
	else
	{
		// Right at the current key
		a_rLerpFromIndex = a_rCurrentKeyIndex;
		a_rLerpToIndex   = a_rCurrentKeyIndex;
		return 0.0f;
	}

	TUINT16 uiLerpFromTime = *GetKey( a_rLerpFromIndex );
	TUINT16 uiLerpToTime   = *GetKey( a_rLerpToIndex );

	return ( TUINT16( a_uiCurrentAnimTime - uiLerpFromTime ) * ( 1.0f / 65535 ) ) / ( TUINT16( uiLerpToTime - uiLerpFromTime ) * ( 1.0f / 65535 ) );
}

// $deBlob: FUNCTION 0068eac0
// $Barnyard: FUNCTION 006ca130
void TSkeletonSequenceBone::IntraBlend( TKeyframeLibraryInstance* a_pKeyLib, TFLOAT a_fProgress, TUINT16& a_rFromKey, TUINT16& a_rToKey, TQuaternion& a_rOutRotation, TVector4& a_rOutPosition )
{
	TASSERT( a_rFromKey <= TMath::TUINT16_MAX );
	TUINT16* pFromKey = GetKey( a_rFromKey );
	TUINT16* pToKey   = GetKey( a_rToKey );

	TQuaternion::Slerp( a_rOutRotation, *a_pKeyLib->GetQ( pFromKey[ 1 ] ), *a_pKeyLib->GetQ( pToKey[ 1 ] ), a_fProgress );

	// NOTE: retail passes the (12 byte) translations to TVector4::Lerp3 as they are, w isn't used
	if ( IsTranslateAnimated() )
		a_rOutPosition.Lerp3( *a_pKeyLib->GetT( pFromKey[ 2 ] ), *a_pKeyLib->GetT( pToKey[ 2 ] ), a_fProgress );
}

// $deBlob: FUNCTION 0068f1d0
// $Barnyard: FUNCTION 006ca410
TUINT16 TSkeleton::GetBoneTransformLocal( TINT a_iBone, TINT a_iSequence, TFLOAT a_fTime, TUINT16 a_uiCurrentKey, TMatrix44& a_rOutTransform )
{
	TSkeletonSequence*     pSequence = GetSequence( a_iSequence );
	TSkeletonSequenceBone* pSeqBone  = pSequence->GetBone( a_iBone );

	TUINT16 uiFromKey;
	TUINT16 uiToKey;
	TFLOAT  fProgress = pSeqBone->GetKeyPair( TUINT16( TINT( a_fTime / pSequence->GetDuration() * 65535.0f ) ), a_uiCurrentKey, uiFromKey, uiToKey );

	if ( pSeqBone->GetKeyCount() != 0 )
	{
		TQuaternion quatRotation;
		TVector4    vPosition;
		pSeqBone->IntraBlend( &m_KeyLibraryInstance, fProgress, uiFromKey, uiToKey, quatRotation, vPosition );

		if ( !pSeqBone->IsTranslateAnimated() )
		{
			TSkeletonBone* pBone = GetBone( a_iBone );
			vPosition            = TVector4( pBone->GetPosition().x, pBone->GetPosition().y, pBone->GetPosition().z, 1.0f );
		}

		a_rOutTransform.SetFromQuaternion( quatRotation );
		a_rOutTransform.GetTranslation3() = vPosition.AsVector3();
	}
	else
	{
		TSkeletonBone* pBone = GetBone( a_iBone );
		a_rOutTransform.SetFromQuaternion( pBone->GetRotation() );
		a_rOutTransform.GetTranslation3() = pBone->GetPosition();
	}

	return a_uiCurrentKey;
}

// $deBlob: FUNCTION 0068f3b0
// $Barnyard: FUNCTION 006ca550
void TSkeleton::GetBoneTransform( TINT a_iBone, TINT a_iSequence, TFLOAT a_fTime, TMatrix44& a_rOutTransform )
{
	TINT iParentBone = GetBone( a_iBone )->GetParentBone();

	if ( iParentBone == TBONE_INVALID )
	{
		GetBoneTransformLocal( a_iBone, a_iSequence, a_fTime, 0, a_rOutTransform );
		return;
	}

	TMatrix44 matParent;
	TMatrix44 matLocal;
	GetBoneTransform( iParentBone, a_iSequence, a_fTime, matParent );
	GetBoneTransformLocal( a_iBone, a_iSequence, a_fTime, 0, matLocal );
	a_rOutTransform.Multiply( matParent, matLocal );
}

TOSHI_NAMESPACE_END
