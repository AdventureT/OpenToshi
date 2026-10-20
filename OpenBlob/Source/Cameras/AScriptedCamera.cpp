#include "pch.h"
#include "AScriptedCamera.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"

#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The bones' space to the world's (Y and Z swapped)
static const TMatrix44 s_mBoneToWorld = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

// $deBlob: FUNCTION 005942d0
// $deBlob: FUNCTION 005942e0
// $deBlob: FUNCTION 00594350
// $deBlob: FUNCTION 005943b0
// $deBlob: FUNCTION 005943c0
// $deBlob: FUNCTION 0053b9f0
// $deBlob: FUNCTION 00857fa0
TDEFINE_CLASS( AScriptedCamera );

// $deBlob: FUNCTION 005943d0
AScriptedCamera::AScriptedCamera()
    : ACamera( CT_SCRIPTED )
{
	m_iSequence      = -1;
	m_pModelInstance = TNULL;
	m_bPlaying       = TFALSE;
	m_pAnimation     = TNULL;
	m_fBlendOutStart = 0.0f;
	m_fBlendOutEnd   = 1.0f;
	m_aiBones[ 0 ]   = -1;
	m_aiBones[ 1 ]   = -1;
}

// $deBlob: FUNCTION 00594430
// $deBlob: FUNCTION 00594950
AScriptedCamera::~AScriptedCamera()
{
	DeinitialiseActor();
}

// $deBlob: FUNCTION 00594490
void AScriptedCamera::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bPlaying )
		return;

	m_pModelInstance->UpdateImp( a_fDeltaTime );

	if ( IsAnimating() )
	{
		TSkeletonInstance* pSkeletonInstance = m_pModelInstance->GetAnimInterface()->GetSkeletonInstance();

		if ( pSkeletonInstance->GetOverlayAnimationCount() + pSkeletonInstance->GetBaseAnimationCount() != 0 )
		{
			TSkeletonSequence* pSequence = m_pAnimation->GetSequencePtr();

			// Holds the last frame
			if ( m_pAnimation->GetTotalTime() > pSequence->GetDuration() )
				m_pAnimation->SetTime( pSequence->GetDuration() );

			pSkeletonInstance->UpdateState( TFALSE );
		}

		TMatrix44 mMatrix;
		GetBoneMatrix( mMatrix, 0 );
		SetMatrix( mMatrix );

		if ( m_pModelInstance->GetAnimInterface()->IsAnimationFinished( m_iSequence ) )
		{
			m_pModelInstance->GetAnimInterface()->StopAllAnims( 0.0f );
			m_pAnimation = TNULL;
			m_bPlaying   = TFALSE;
		}
	}

	ACamera::Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 005945c0
TBOOL AScriptedCamera::InitialiseActor( const TCHAR* a_szFileName, const TCHAR* a_szBone, const TCHAR* a_szBone2 )
{
	DeinitialiseActor();

	m_pModelInstance = AModelFactory::GetSingleton()->CreateUnmanagedInstance( a_szFileName, TTRUE, 0, TTRUE );
	TASSERT( m_pModelInstance != TNULL );

	m_aiBones[ 0 ] = TINT16( m_pModelInstance->FindBone( a_szBone ) );

	if ( a_szBone2 != TNULL )
		m_aiBones[ 1 ] = TINT16( m_pModelInstance->FindBone( a_szBone2 ) );

	return TTRUE;
}

// $deBlob: FUNCTION 00594650
void AScriptedCamera::DeinitialiseActor()
{
	if ( m_pModelInstance != TNULL )
	{
		AModelFactory::GetSingleton()->DestroyInstance( m_pModelInstance );
		m_pModelInstance = TNULL;
	}

	m_bPlaying       = TFALSE;
	m_iSequence      = -1;
	m_pAnimation     = TNULL;
	m_aiBones[ 0 ]   = -1;
	m_aiBones[ 1 ]   = -1;
	m_fBlendOutStart = 0.0f;
	m_fBlendOutEnd   = 1.0f;
}

// $deBlob: FUNCTION 005946b0
TBOOL AScriptedCamera::PlayAnimation( const TCHAR* a_szName )
{
	m_iSequence = TINT16( m_pModelInstance->GetAnimInterface()->GetAnimID( a_szName ) );

	if ( m_iSequence != -1 )
	{
		m_pAnimation = m_pModelInstance->GetAnimInterface()->AddAnim( m_iSequence, 0.0f, 1.0f, 1.0f );

		if ( m_pAnimation != TNULL )
		{
			m_bPlaying = TTRUE;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00594730
TBOOL AScriptedCamera::IsAnimating()
{
	if ( m_iSequence == -1 )
		return TFALSE;

	TSkeletonInstance* pSkeletonInstance = m_pModelInstance->GetAnimInterface()->GetSkeletonInstance();
	return ( pSkeletonInstance != TNULL ? pSkeletonInstance->IsAnimating( m_iSequence ) : TNULL ) != TNULL;
}

// $deBlob: FUNCTION 00594770
TFLOAT AScriptedCamera::GetAnimationLength()
{
	if ( m_pAnimation == TNULL )
		return 0.0f;

	return m_pAnimation->GetSequencePtr()->GetDuration();
}

// $deBlob: FUNCTION 005947d0
TFLOAT AScriptedCamera::GetAnimationTime()
{
	if ( m_pAnimation == TNULL )
		return 0.0f;

	return m_pAnimation->GetTotalTime();
}

// $deBlob: FUNCTION 005947f0
void AScriptedCamera::StopAnimation()
{
	m_pModelInstance->GetAnimInterface()->StopAllAnims( 0.0f );
	m_pAnimation = TNULL;
	m_bPlaying   = TFALSE;
}

// $deBlob: FUNCTION 00594820
TBOOL AScriptedCamera::GetBoneMatrix( TMatrix44& a_rOutMatrix, TINT a_iBone )
{
	if ( m_aiBones[ a_iBone ] < 0 )
	{
		a_rOutMatrix = TMatrix44::IDENTITY;
		return TFALSE;
	}

	TMatrix44 mBone;
	m_pModelInstance->GetTransform( a_rOutMatrix );
	m_pModelInstance->GetBoneTransformLocal( m_aiBones[ a_iBone ], mBone );

	TMatrix44 mTemp = a_rOutMatrix;
	a_rOutMatrix.Multiply( mTemp, mBone );

	mTemp = a_rOutMatrix;
	a_rOutMatrix.Multiply( mTemp, s_mBoneToWorld );

	return TFALSE;
}

// $deBlob: FUNCTION 00594930
void AScriptedCamera::SetTransform( const TMatrix44& a_rcMatrix )
{
	if ( m_pModelInstance != TNULL )
		m_pModelInstance->SetTransform( a_rcMatrix );
}
