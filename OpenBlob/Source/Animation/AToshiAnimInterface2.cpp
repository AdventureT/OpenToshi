#include "pch.h"
#include "AToshiAnimInterface2.h"
#include "AAnimationPose.h"

#include <Render/TAnimation.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00574230
AToshiAnimInterface2::AToshiAnimInterface2()
{
	m_pSkeletonInstance = TNULL;
}

// $deBlob: FUNCTION 00574240
// $deBlob: FUNCTION 00572c50
AToshiAnimInterface2::~AToshiAnimInterface2()
{
}

// $deBlob: FUNCTION 00574250
void AToshiAnimInterface2::SetPose( AAnimationPose* a_pPose, TFLOAT a_fBlendInSpeed, TFLOAT a_fSpeedMult )
{
	if ( m_pSkeletonInstance == TNULL )
		return;

	for ( TINT i = 0; i < a_pPose->GetComponentCount(); i++ )
	{
		AAnimationPose::Component& rComponent = a_pPose->GetComponent( i );

		if ( rComponent.iSequence < 0 )
		{
			// An invalid component resets the skeleton
			SetInvalidPose();
			return;
		}

		TAnimation* pAnimation = m_pSkeletonInstance->IsAnimating( TINT16( rComponent.iSequence ) );

		if ( pAnimation == TNULL )
		{
			pAnimation = m_pSkeletonInstance->AddAnimation( TUINT16( rComponent.iSequence ), rComponent.fWeight, a_fBlendInSpeed );

			if ( pAnimation )
			{
				pAnimation->SetSpeed( rComponent.fSpeed * a_fSpeedMult );
				pAnimation->SetTime( rComponent.fTime );
				pAnimation->SetState( TAnimation::STATE_BLENDING_IN );
			}
		}
		else
		{
			pAnimation->SetSpeed( rComponent.fSpeed * a_fSpeedMult );
			pAnimation->SetWeight( rComponent.fWeight );
		}
	}
}

// $deBlob: FUNCTION 005743a0
void AToshiAnimInterface2::SetInvalidPose()
{
	if ( m_pSkeletonInstance )
	{
		m_pSkeletonInstance->RemoveAllAnimations( 0.0f );
		m_pSkeletonInstance->SetStateFromBasePose();
	}
}

// $deBlob: FUNCTION 005743c0
TFLOAT AToshiAnimInterface2::GetPoseLength( AAnimationPose* a_pPose )
{
	if ( m_pSkeletonInstance == TNULL )
		return 0.0f;

	TFLOAT fLength = 0.0f;

	for ( TINT i = 0; i < a_pPose->GetComponentCount(); i++ )
	{
		TINT iSequence = a_pPose->GetComponent( i ).iSequence;

		if ( iSequence >= 0 )
		{
			TSkeletonSequence* pSequence = m_pSkeletonInstance->GetSkeleton()->GetSequence( iSequence );

			if ( pSequence && pSequence->GetDuration() > fLength )
				fLength = pSequence->GetDuration();
		}
	}

	return fLength;
}

// $deBlob: FUNCTION 005744e0
TAnimation* AToshiAnimInterface2::AddAnim( TINT16 a_iSequence, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed )
{
	if ( a_iSequence >= 0 && m_pSkeletonInstance )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->AddAnimation( a_iSequence, a_fWeight, a_fBlendInSpeed );

		if ( pAnimation )
		{
			pAnimation->SetSpeed( a_fSpeed );
			return pAnimation;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00574530
void AToshiAnimInterface2::StopAnim( TINT16 a_iSequence, TFLOAT a_fBlendOutSpeed )
{
	if ( m_pSkeletonInstance )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->IsAnimating( a_iSequence );

		if ( pAnimation )
			m_pSkeletonInstance->RemoveAnimation( pAnimation, a_fBlendOutSpeed );
	}
}

// $deBlob: FUNCTION 00574560
void AToshiAnimInterface2::StopAllAnims( TFLOAT a_fBlendOutSpeed )
{
	if ( m_pSkeletonInstance )
		m_pSkeletonInstance->RemoveAllAnimations( a_fBlendOutSpeed );
}

// $deBlob: FUNCTION 00574580
TBOOL AToshiAnimInterface2::IsAnimationFinished( TINT16 a_iSequence )
{
	TAnimation* pAnimation = m_pSkeletonInstance->IsAnimating( a_iSequence );
	TASSERT( pAnimation, "Animation is not playing" );

	TFLOAT fTotalTime = pAnimation->GetTotalTime();
	if ( pAnimation->GetSkeletonInstance()->GetSkeleton()->GetSequence( pAnimation->GetSequence() )->GetDuration() <= fTotalTime )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 00574630
TFLOAT AToshiAnimInterface2::GetAnimationLength( TINT16 a_iSequence )
{
	if ( m_pSkeletonInstance )
	{
		TSkeletonSequence* pSequence = m_pSkeletonInstance->GetSkeleton()->GetSequence( a_iSequence );

		if ( pSequence )
			return pSequence->GetDuration();
	}

	return 0.0f;
}

// $deBlob: FUNCTION 005746a0
TFLOAT AToshiAnimInterface2::GetAnimationTotalTime( TINT16 a_iSequence )
{
	if ( m_pSkeletonInstance )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->IsAnimating( a_iSequence );

		if ( pAnimation )
			return pAnimation->GetTotalTime();
	}

	return 0.0f;
}

// $deBlob: FUNCTION 005746d0
TFLOAT AToshiAnimInterface2::GetAnimationSeqTime( TINT16 a_iSequence )
{
	if ( m_pSkeletonInstance )
	{
		TAnimation* pAnimation = m_pSkeletonInstance->IsAnimating( a_iSequence );

		if ( pAnimation )
			return pAnimation->GetSeqTime();
	}

	return 0.0f;
}

// $deBlob: FUNCTION 00574700
TINT16 AToshiAnimInterface2::GetAnimID( const TCHAR* a_szName )
{
	if ( m_pSkeletonInstance == TNULL )
		return -1;

	return m_pSkeletonInstance->GetSkeleton()->GetSequenceID( a_szName, -1 );
}

// $deBlob: FUNCTION 00574730
void AToshiAnimInterface2::SetSkeletonInstance( TSkeletonInstance* a_pSkeletonInstance )
{
	m_pSkeletonInstance = a_pSkeletonInstance;
}
