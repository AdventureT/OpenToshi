#include "pch.h"
#include "AUVAnimation.h"
#include "ARandom.h"

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005fd0a0
AUVAnimationPool::AUVAnimationPool( TINT a_iSize, const TCHAR* a_szName )
{
	m_szName = a_szName;
	m_iSize  = a_iSize;
	TASSERT( m_iSize > 0 );

	// NOTE: retail allocates the array with TMemalign16
	m_pAnimations = new AUVAnimation[ m_iSize ];

	for ( TINT i = 0; i < m_iSize; i++ )
	{
		m_pAnimations[ i ].m_pOwnerPool = this;
		m_lFree.PushBack( &m_pAnimations[ i ] );
	}
}

// $deBlob: FUNCTION 005fd1c0
// $deBlob: FUNCTION 0061d990
AUVAnimationPool::~AUVAnimationPool()
{
	TASSERT( m_lUsed.IsEmpty() );
	m_lUsed.Clear();
	m_lFree.Clear();

	if ( m_pAnimations != TNULL )
		delete[] m_pAnimations;

	m_pAnimations = TNULL;
}

// $deBlob: FUNCTION 005fd290
AUVAnimation* AUVAnimationPool::Alloc()
{
	TASSERT( !m_lFree.IsEmpty() );
	AUVAnimation* pUVAnim = m_lFree.Head();
	pUVAnim->Remove();
	m_lUsed.PushBack( pUVAnim );

	return pUVAnim;
}

// $deBlob: FUNCTION 005fd2f0
void AUVAnimationPool::Free( AUVAnimation* a_pUVAnim )
{
	TASSERT( a_pUVAnim->m_pOwnerPool->m_lUsed.IsInList( a_pUVAnim ) );
	a_pUVAnim->Remove();
	a_pUVAnim->m_pOwnerPool->m_lFree.PushBack( a_pUVAnim );
}

// $deBlob: FUNCTION 005fd480
AUVAnimation::AUVAnimation()
{
	m_pOwnerPool = TNULL;
	m_eType      = TYPE_UNSET;
	m_iLastFrame = 0;
}

// $deBlob: FUNCTION 005fd350
void AUVAnimation::SetupOffset()
{
	m_fUOffset = 0.0f;
	m_fVOffset = 0.0f;
	m_fUnk14   = 0.0f;
	m_fUnk18   = 0.0f;
	m_eType    = TYPE_UVOFFSET;
}

// $deBlob: FUNCTION 005fd380
void AUVAnimation::SetupUnk1( TBOOL a_bUnk )
{
	m_bUnk0C   = a_bUnk;
	m_fVOffset = 0.0f;
	m_fUnk14   = 0.0f;
	m_eType    = TYPE_SPRITE;
}

// $deBlob: FUNCTION 005fd3b0
void AUVAnimation::SetupNone()
{
	m_eType = TYPE_NONE;
}

// $deBlob: FUNCTION 005fd3c0
void AUVAnimation::Update( TUINT32 a_uiUFunc, TUINT32 a_uiVFunc, TFLOAT a_fUParam1, TFLOAT a_fUParam2, TFLOAT a_fVParam1, TFLOAT a_fVParam2, TFLOAT& a_rfUOffset, TFLOAT& a_rfVOffset, TFLOAT a_fTime, TINT a_iFrame, TBOOL a_bUseCache )
{
	if ( m_eType == TYPE_UVOFFSET )
		UpdateUVOffset( a_uiUFunc, a_uiVFunc, a_fUParam1, a_fUParam2, a_fVParam1, a_fVParam2, a_rfUOffset, a_rfVOffset, a_fTime, a_iFrame, a_bUseCache );
	else if ( m_eType == TYPE_SPRITE )
		UpdateSprite( a_uiUFunc, a_uiVFunc, a_fUParam1, a_fUParam2, a_fVParam1, a_fVParam2, a_rfUOffset, a_rfVOffset, a_fTime, a_iFrame );
}

// The offset of one axis: 1 scrolls (sawtooth), 2 is a sine, 3 a triangle, 4 a square wave; returns TFALSE for the
// other functions (the offset is kept)
TBOOL AUVAnimation::EvaluateFunction( TUINT32 a_uiFunc, TFLOAT a_fParam1, TFLOAT& a_rfParam2, TFLOAT a_fTime )
{
	switch ( a_uiFunc )
	{
		case 1:
			a_rfParam2 = Sawtooth( a_fTime, a_fParam1 );
			return TTRUE;
		case 2:
			a_rfParam2 = TFLOAT( sin( TDOUBLE( TFLOAT( fmod( TDOUBLE( a_fTime ), TDOUBLE( a_fParam1 ) ) ) ) ) ) * a_rfParam2;
			return TTRUE;
		case 3:
			a_rfParam2 = Triangle( a_fTime, a_fParam1, a_rfParam2 );
			return TTRUE;
		case 4:
			if ( 0.5f <= TFLOAT( fmod( TDOUBLE( a_fTime ), TDOUBLE( a_fParam1 ) ) ) )
				a_rfParam2 = -a_rfParam2;
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005fd4a0
void AUVAnimation::UpdateUVOffset( TUINT32 a_uiUFunc, TUINT32 a_uiVFunc, TFLOAT a_fUParam1, TFLOAT a_fUParam2, TFLOAT a_fVParam1, TFLOAT a_fVParam2, TFLOAT& a_rfUOffset, TFLOAT& a_rfVOffset, TFLOAT a_fTime, TINT a_iFrame, TBOOL a_bUseCache )
{
	TASSERT( m_eType == TYPE_UVOFFSET && "m_Type==UVOffset" );

	if ( m_iLastFrame == a_iFrame && a_bUseCache )
	{
		a_rfUOffset = m_fUOffset;
		a_rfVOffset = m_fVOffset;
		return;
	}

	m_iLastFrame = a_iFrame;

	if ( EvaluateFunction( a_uiUFunc, a_fUParam1, a_fUParam2, a_fTime ) )
	{
		m_fUOffset  = a_fUParam2;
		a_rfUOffset = a_fUParam2;
	}

	TASSERT( -1.0f <= a_rfUOffset );
	TASSERT( 1.0f >= a_rfUOffset );

	if ( EvaluateFunction( a_uiVFunc, a_fVParam1, a_fVParam2, a_fTime ) )
	{
		m_fVOffset  = a_fVParam2;
		a_rfVOffset = a_fVParam2;
	}

	TASSERT( -1.0f <= a_rfVOffset );
	TASSERT( 1.0f >= a_rfVOffset );
}

// $deBlob: FUNCTION 005fd840
void AUVAnimation::UpdateSprite( TINT a_iNumColumns, TINT a_iNumRows, TFLOAT a_fFPS, TFLOAT a_fUnused1, TFLOAT a_fUnused2, TFLOAT a_fUnused3, TFLOAT& a_rfUOffset, TFLOAT& a_rfVOffset, TFLOAT a_fTime, TUINT a_uiFrame )
{
	TASSERT( m_eType == TYPE_SPRITE && "m_Type==Sprite" );

	if ( TUINT( m_iLastFrame ) < a_uiFrame )
	{
		if ( a_fTime < m_fSpriteTime )
			m_fSpriteTime = 0.0f;

		a_fTime -= m_fSpriteTime;

		TINT iTotalFrames = a_iNumColumns * a_iNumRows;
		TASSERT( 0 < iTotalFrames );

		TFLOAT fFrameTime = 1.0f / a_fFPS;

		while ( fFrameTime <= a_fTime )
		{
			a_fTime -= fFrameTime;
			m_fSpriteTime += fFrameTime;

			if ( !m_bUnk0C )
			{
				m_iSpriteFrame = ( m_iSpriteFrame + 1 ) % iTotalFrames;
			}
			else
			{
				TINT iNewFrame;

				do
				{
					TASSERT( iTotalFrames > 0 && "a_iUpper>a_iLower" );
					iNewFrame = ARandom::GetSingleton()->GetRandom().GetInt( iTotalFrames );
				} while ( iNewFrame == m_iSpriteFrame );

				m_iSpriteFrame = iNewFrame;
			}
		}
	}

	a_rfUOffset = TFLOAT( m_iSpriteFrame % a_iNumColumns ) * ( 1.0f / TFLOAT( a_iNumColumns ) );
	TASSERT( 0.0f <= a_rfUOffset );
	TASSERT( 1.0f >= a_rfUOffset );

	a_rfVOffset = TFLOAT( m_iSpriteFrame / a_iNumColumns ) * ( 1.0f / TFLOAT( a_iNumRows ) );
	TASSERT( 0.0f <= a_rfVOffset );
	TASSERT( 1.0f >= a_rfVOffset );

	m_iLastFrame = a_uiFrame;
}

// $deBlob: FUNCTION 005fdae0
TFLOAT AUVAnimation::Triangle( TFLOAT a_fTime, TFLOAT a_fPeriod, TFLOAT a_fAmplitude )
{
	TFLOAT fPhase = TFLOAT( fmod( TDOUBLE( a_fTime ), TDOUBLE( a_fPeriod ) ) );

	if ( fPhase < 0.25f )
		return fPhase * 4.0f * a_fAmplitude;

	if ( fPhase < 0.5f )
		return ( 2.0f - fPhase * 4.0f ) * a_fAmplitude;

	if ( fPhase < 0.75f )
		return -( ( fPhase * 4.0f - 2.0f ) * a_fAmplitude );

	return -( ( 4.0f - fPhase * 4.0f ) * a_fAmplitude );
}

// $deBlob: FUNCTION 005fde40
TFLOAT AUVAnimation::Sine( TFLOAT a_fTime, TFLOAT a_fPeriod, TFLOAT a_fAmplitude )
{
	return TFLOAT( sin( fmod( TDOUBLE( a_fTime ), TDOUBLE( a_fPeriod ) ) ) ) * a_fAmplitude;
}

// $deBlob: FUNCTION 005fdea0
TFLOAT AUVAnimation::Square( TFLOAT a_fTime, TFLOAT a_fPeriod, TFLOAT a_fAmplitude )
{
	if ( fmod( TDOUBLE( a_fTime ), TDOUBLE( a_fPeriod ) ) >= 0.5 )
		return -a_fAmplitude;

	return a_fAmplitude;
}

// $deBlob: FUNCTION 005fddc0
TFLOAT AUVAnimation::Sawtooth( TFLOAT a_fTime, TFLOAT a_fFrequency )
{
	if ( a_fFrequency == 0.0f )
		return 0.0f;

	return TFLOAT( fmod( TDOUBLE( a_fTime ), TDOUBLE( 1.0f / a_fFrequency ) ) ) * a_fFrequency;
}
