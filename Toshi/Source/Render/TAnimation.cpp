#include "ToshiPCH.h"
#include "TAnimation.h"
#include "TSkeleton.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0068e0a0
// $Barnyard: FUNCTION 006caea0
TBOOL TAnimation::UpdateTime( TFLOAT a_fDeltaTime )
{
	TSkeleton*         pSkeleton = m_pSkeletonInstance->GetSkeleton();
	TSkeletonSequence* pSeq      = pSkeleton->GetSequence( m_iSeqID );

	TFLOAT fSeqDuration = pSeq->GetDuration();

	// The total time always grows, even when playing backwards
	m_fTotalTime += TMath::Abs( m_fSpeed * a_fDeltaTime );
	m_fSeqTime += m_fSpeed * a_fDeltaTime;

	if ( pSeq->GetMode() == TSkeletonSequence::MODE_LOOPED && !HASANYFLAG( m_eFlags, Flags_Managed ) )
	{
		if ( 0.0f <= m_fSpeed || 0.0f <= m_fSeqTime )
		{
			if ( fSeqDuration <= m_fSeqTime )
			{
				// Make m_fSeqTime less than animation duration
				m_fSeqTime -= TMath::FloorToInt( m_fSeqTime / fSeqDuration ) * pSeq->GetDuration();

				// Reset the current keys of all bones
				TUtil::MemSet( GetBones(), 0, pSkeleton->GetAutoBoneCount() * sizeof( TAnimationBone ) );
			}
		}
		else
		{
			m_fSeqTime = fSeqDuration;
		}
	}
	else
	{
		if ( m_fSeqTime < 0.0f && m_fSpeed < 0.0f )
			m_fSeqTime = fSeqDuration;

		if ( m_fSeqTime >= 0.0f )
		{
			if ( m_fSeqTime > fSeqDuration )
				m_fSeqTime = fSeqDuration;
		}
		else
		{
			m_fSeqTime = 0.0f;
		}
	}

	if ( m_eState == STATE_BLENDING_IN )
	{
		if ( m_fBlendInSpeed == 0.0f )
			m_fWeight = m_fDestWeight;
		else
			m_fWeight = ( 1.0f / m_fBlendInSpeed ) * a_fDeltaTime + m_fWeight;

		if ( m_fWeight >= m_fDestWeight )
		{
			m_fWeight = m_fDestWeight;
			m_eState  = STATE_PLAYING;
		}

		return TTRUE;
	}
	else if ( m_eState == STATE_PLAYING )
	{
		if ( !HASANYFLAG( m_eFlags, Flags_Managed ) )
			return TTRUE;

		if ( m_fTotalTime >= fSeqDuration )
			m_eState = STATE_BLENDING_OUT;

		return TTRUE;
	}
	else if ( m_eState == STATE_BLENDING_OUT )
	{
		if ( m_fBlendOutSpeed == 0.0f )
			m_fWeight = -1.0f;
		else
			m_fWeight = m_fWeight - ( 1.0f / m_fBlendOutSpeed ) * a_fDeltaTime;

		if ( m_fWeight <= 0.0f )
		{
			// NOTE: the flag is read before RemoveAnimation clears the flags (Barnyard reads it after)
			TBOOL bUpdateStateOnRemove = HASANYFLAG( m_eFlags, Flags_UpdateStateOnRemove );
			m_pSkeletonInstance->RemoveAnimation( this, 0.0f );
			return bUpdateStateOnRemove;
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0068e010
// $Barnyard: FUNCTION 006cb0c0
void TAnimation::RemoveAnimation( TFLOAT a_fVal )
{
	if ( m_pSkeletonInstance )
		m_pSkeletonInstance->RemoveAnimation( this, a_fVal );
}

// $deBlob: FUNCTION 0068e040
void TAnimation::MoveAnimationToFront()
{
	if ( m_pSkeletonInstance )
		m_pSkeletonInstance->MoveAnimationToFront( this );
}

// $deBlob: FUNCTION 0068e050
void TAnimation::MoveAnimationToBack()
{
	if ( m_pSkeletonInstance )
		m_pSkeletonInstance->MoveAnimationToBack( this );
}

// $deBlob: FUNCTION 0068dd80
TFLOAT TAnimation::SetWeight( TFLOAT a_fWeight )
{
	TFLOAT fOldValue = m_fWeight;

	m_fWeight = a_fWeight;
	m_eState  = STATE_PLAYING;

	return fOldValue;
}

// $deBlob: FUNCTION 0068ddb0
// $Barnyard: FUNCTION 006ca2d0
TFLOAT TAnimation::SetDestWeight( TFLOAT a_fDestWeight, TFLOAT a_fBlendInSpeed )
{
	TFLOAT fOldValue = m_fDestWeight;

	m_fDestWeight   = a_fDestWeight;
	m_fBlendInSpeed = a_fBlendInSpeed;

	if ( a_fBlendInSpeed <= 0.0f )
	{
		m_fWeight = a_fDestWeight;
		m_eState  = STATE_PLAYING;
	}
	else
	{
		m_eState = STATE_BLENDING_IN;
	}

	return fOldValue;
}

// $deBlob: FUNCTION 0056f2c0
TSkeletonSequence* TAnimation::GetSequencePtr() const
{
	return m_pSkeletonInstance->GetSkeleton()->GetSequence( m_iSeqID );
}

// NOTE: inlined in TSkeleton::CreateInstance
TAnimation::TAnimation()
{
	m_eFlags = Flags_None;
}

TAnimation::~TAnimation()
{
}

// $deBlob: FUNCTION 0068df50
// $Barnyard: FUNCTION 006ca230
void TAnimation::ChangeToManaged( TFLOAT a_fBlendOutSpeed )
{
	if ( IsActive() && !IsManaged() )
	{
		TFLOAT fSeqDuration = m_pSkeletonInstance->GetSkeleton()->GetSequence( m_iSeqID )->GetDuration();

		m_eFlags |= Flags_Managed;
		m_fBlendOutSpeed = a_fBlendOutSpeed;
		m_fSeqTime       = m_fSeqTime - fSeqDuration * TMath::FloorToInt( m_fSeqTime / fSeqDuration );
		m_fTotalTime     = m_fSeqTime;
	}
}

// $deBlob: FUNCTION 0068def0
// $Barnyard: FUNCTION 006ca1d0
TBOOL TAnimation::WillFinishThisFrame( TFLOAT a_fDeltaTime ) const
{
	if ( !IsManaged() || m_eState != STATE_BLENDING_OUT )
		return TFALSE;

	if ( 0.0f == m_fBlendOutSpeed )
		return TTRUE;

	return m_fWeight - 1.0f / m_fBlendOutSpeed * a_fDeltaTime <= 0.0f;
}

// $deBlob: FUNCTION 0068de00
// $Barnyard: FUNCTION 006ca310
TFLOAT TAnimation::SetTime( TFLOAT a_fTime )
{
	TFLOAT             fOldTime  = m_fTotalTime;
	TSkeletonSequence* pSequence = m_pSkeletonInstance->GetSkeleton()->GetSequence( m_iSeqID );

	m_fTotalTime = a_fTime;
	m_fSeqTime   = a_fTime;

	if ( pSequence->GetMode() != TSkeletonSequence::MODE_LOOPED || IsManaged() )
	{
		if ( a_fTime >= 0.0f )
		{
			if ( a_fTime > pSequence->GetDuration() )
				m_fSeqTime = pSequence->GetDuration();
		}
		else
		{
			m_fSeqTime = 0.0f;
		}
	}
	else
	{
		TFLOAT fDuration = pSequence->GetDuration();
		m_fSeqTime       = m_fSeqTime - fDuration * TMath::FloorToInt( a_fTime / fDuration );
	}

	return fOldTime;
}

// $deBlob: FUNCTION 0068f5d0
TINT __stdcall TAnimation::FindMaxBoundingSphereIndex( TQList<TAnimation>& a_rList )
{
	TINT iMax = -1;

	T2_FOREACH( a_rList, it )
	{
		TINT iBoundingSphereIndex = it->GetSkeletonInstance()->GetSkeleton()->GetSequence( it->GetSequence() )->GetBoundingSphereIndex();
		iMax                      = TMath::Max( iMax, iBoundingSphereIndex );
	}

	return iMax;
}

TOSHI_NAMESPACE_END
