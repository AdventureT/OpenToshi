#include "pch.h"
#include "AAIMoveSolver.h"

#include <Math/TMath.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAIMoveSolver ) == 0x28 );
TSTATICASSERT( sizeof( AAIMoveSolver::Constraint ) == 0x18 );

// The weight range of each constraint type: { weight 0, weight 1 } (NormaliseContraints)
static const TFLOAT g_aContraintTypeWeights[ AAIMoveSolver::CONSTRAINTTYPE_NUMOF ][ 2 ] = {
	{ 0.2f, 0.8f },   // CONSTRAINTTYPE_Suggestion
	{ -0.3f, -0.8f }, // CONSTRAINTTYPE_NPC
	{ -1.0f, -1.0f }, // CONSTRAINTTYPE_World
};

// One bucket of Solve: 6 degrees of yaw
struct SolveBucket
{
	TFLOAT                           fSum;  // 0x0
	TFLOAT                           fMax;  // 0x4
	const AAIMoveSolver::Constraint* pBest; // 0x8 the constraint with the heaviest contribution
};

static TCONSTEXPR_VAR TINT SOLVE_NUM_BUCKETS = 60;

// $deBlob: FUNCTION 007ea8e0
AAIMoveSolver::AAIMoveSolver()
    : m_lConstraints( GetGlobalAllocator(), 0x20, 0x20 )
    , m_lTempConstraints( GetGlobalAllocator(), 0x20, 0x20 )
{
}

// $deBlob: FUNCTION 007ea950
AAIMoveSolver::~AAIMoveSolver()
{
}

// $deBlob: FUNCTION 007ea9b0
void AAIMoveSolver::AddConstraint( const Constraint& a_rcConstraint )
{
	m_lConstraints.PushBack( a_rcConstraint );
}

// $deBlob: FUNCTION 007ea9f0
void AAIMoveSolver::Reset()
{
	m_lConstraints.Clear();
}

// $deBlob: FUNCTION 007eaa00
TBOOL AAIMoveSolver::Solve( const ConstraintList& a_rcSuggestions, TFLOAT& a_rfSolution )
{
	// Only one way to go
	if ( m_lConstraints.Size() == 0 && a_rcSuggestions.Size() == 1 )
	{
		a_rfSolution = a_rcSuggestions[ 0 ].m_oArc.m_fCenter;
		return TTRUE;
	}

	if ( a_rcSuggestions.Size() == 0 )
		return TFALSE;

	m_lTempConstraints.Clear();

	for ( TINT i = 0; i < a_rcSuggestions.Size(); i++ )
		m_lTempConstraints.PushBack( a_rcSuggestions[ i ] );

	for ( TINT i = 0; i < m_lConstraints.Size(); i++ )
		m_lTempConstraints.PushBack( m_lConstraints[ i ] );

	NormaliseContraints( m_lTempConstraints );

	SolveBucket aBuckets[ SOLVE_NUM_BUCKETS ];
	TUtil::MemSet( aBuckets, 0, sizeof( aBuckets ) );

	TASSERT( m_lTempConstraints.Size() > 0 );

	// The falloff per bucket away from the arc's center
	static TFLOAT s_fFalloff = 0.0016666667f;

	for ( TINT i = 0; i < m_lTempConstraints.Size(); i++ )
	{
		const Constraint* pCur = &m_lTempConstraints[ i ];

		TFLOAT fHalfSpan = pCur->m_oArc.m_fSpan * 0.5f;
		TINT   iNumSteps = TINT( ( pCur->m_oArc.m_fSpan * 60.0f ) / 360.0f );
		TFLOAT fStart    = pCur->m_oArc.m_fCenter - fHalfSpan;
		TINT   iOffset   = TINT( TFLOAT( TINT( ( fHalfSpan * 60.0f ) / 360.0f + 0.5f ) ) );

		if ( fStart < 0.0f )
			fStart += 360.0f;

		TINT iStart = TINT( ( fStart * 60.0f ) / 360.0f );

		for ( TINT j = 0; j <= iNumSteps; j++, iOffset-- )
		{
			TFLOAT fWeight = pCur->m_fWeight;

			if ( fWeight > 0.0f )
			{
				TFLOAT fFalloff = TFLOAT( iOffset < 0 ? -iOffset : iOffset ) * s_fFalloff;

				// Steering away from what blocks the move falls off slower
				if ( ( iOffset > 0 && ( pCur->m_uiSide & SIDE_Left ) ) || ( iOffset < 0 && ( pCur->m_uiSide & SIDE_Right ) ) )
					fFalloff *= 0.8f;

				fWeight = fWeight - fWeight * fFalloff;
			}

			SolveBucket& rBucket = aBuckets[ ( j + iStart ) % SOLVE_NUM_BUCKETS ];
			rBucket.fSum += fWeight;

			if ( fWeight > rBucket.fMax )
			{
				rBucket.fMax  = fWeight;
				rBucket.pBest = pCur;
			}
		}
	}

	TINT   iBest = -1;
	TFLOAT fBest = 0.0f;

	for ( TINT i = 0; i < SOLVE_NUM_BUCKETS; i++ )
	{
		if ( fBest < aBuckets[ i ].fSum )
		{
			iBest = i;
			fBest = aBuckets[ i ].fSum;
		}
	}

	if ( iBest == -1 )
		return TFALSE;

	TFLOAT fYaw  = TFLOAT( iBest * 6 );
	a_rfSolution = fYaw;

	// The exact center of the best constraint when it's inside the bucket
	TFLOAT fCenter = aBuckets[ iBest ].pBest->m_oArc.m_fCenter;

	if ( fYaw < fCenter && fCenter <= fYaw + 6.0f )
		a_rfSolution = fCenter;

	return TTRUE;
}

// $deBlob: FUNCTION 007eb2d0
void AAIMoveSolver::NormaliseContraints( ConstraintList& a_rConstraints )
{
	for ( TINT i = 0; i < a_rConstraints.Size(); i++ )
	{
		Constraint* pCur = &a_rConstraints[ i ];

		TFLOAT fWeight0 = g_aContraintTypeWeights[ pCur->m_eType ][ 0 ];
		TFLOAT fWeight1 = g_aContraintTypeWeights[ pCur->m_eType ][ 1 ];

		TASSERT( pCur->m_fWeight >= 0.0f );

		if ( pCur->m_fWeight > 0.0f )
			pCur->m_fWeight = ( fWeight1 - fWeight0 ) * pCur->m_fWeight + fWeight0;

		if ( pCur->m_oArc.m_fCenter < 0.0f )
			pCur->m_oArc.m_fCenter += 360.0f;

		if ( pCur->m_oArc.m_fCenter >= 360.0f )
			pCur->m_oArc.m_fCenter -= 360.0f;

		TASSERT( pCur->m_oArc.m_fCenter >= 0 && pCur->m_oArc.m_fCenter < 360 );
		TASSERT( pCur->m_oArc.m_fSpan >= 1 && pCur->m_oArc.m_fSpan < 360 );
	}
}

// $deBlob: FUNCTION 007eafb0
void AAIMoveSolver::AssertYawConversion( TINT a_iYaw, TVector4 a_vOriginal )
{
	a_vOriginal.NormaliseXZ();

	TFLOAT fZ = TFLOAT( cos( TFLOAT( a_iYaw ) * 0.017453292f ) );
	TFLOAT fX = TFLOAT( sin( TFLOAT( a_iYaw ) * 0.017453292f ) );

	TASSERT( TMath::Abs( fX - a_vOriginal.x ) <= 0.01f );
	TASSERT( TMath::Abs( fZ - a_vOriginal.z ) <= 0.01f );
}

// $deBlob: FUNCTION 007eb0c0
TINT AAIMoveSolver::DirectionToYaw( const TVector4& a_rcDirection )
{
	static const TVector4 s_vForward( 0.0f, 0.0f, 1.0f, 1.0f );

	// NOTE: retail rounds to the nearest (x87/SSE default rounding)
	TINT iYaw = TINT( lrintf( TMath::AngleBetweenXZ( s_vForward, a_rcDirection ) * 57.295776f ) );
	WrapYaw( iYaw );

	return iYaw;
}

// $deBlob: FUNCTION 007eb100
TVector4 AAIMoveSolver::YawToDirection( TINT a_iYaw )
{
	TASSERT( a_iYaw >= 0 && a_iYaw < 360 );

	return TVector4(
	    TFLOAT( sin( double( TFLOAT( a_iYaw ) * 0.017453292f ) ) ),
	    0.0f,
	    TFLOAT( cos( double( TFLOAT( a_iYaw ) * 0.017453292f ) ) ),
	    1.0f
	);
}

// $deBlob: FUNCTION 007eb1a0
TVector4 AAIMoveSolver::YawToDirection( TFLOAT a_fYaw )
{
	TASSERT( a_fYaw >= 0 && a_fYaw < 360 );

	return TVector4(
	    TFLOAT( sin( double( a_fYaw * 0.017453292f ) ) ),
	    0.0f,
	    TFLOAT( cos( double( a_fYaw * 0.017453292f ) ) ),
	    1.0f
	);
}

// $deBlob: FUNCTION 007eb240
TFLOAT AAIMoveSolver::YawDelta( TFLOAT a_fYaw1, TFLOAT a_fYaw2 )
{
	TFLOAT fDelta = TMath::Max( a_fYaw1, a_fYaw2 ) - TMath::Min( a_fYaw1, a_fYaw2 );
	TASSERT( fDelta >= 0 );

	if ( fDelta <= 180.0f )
		return fDelta;

	return 360.0f - fDelta;
}
