#include "pch.h"
#include "ABuildPathJob.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABuildPathJob ) == 0x64 );

// $deBlob: FUNCTION 007dd770
TBOOL ABuildPathJob::Run()
{
	const TVector4* pFacing = m_bHasFacing ? &m_vFacing : TNULL;

	switch ( m_eStage )
	{
		case STAGE_CheckGround:
			if ( !m_pNavigator->BuildPathCheckGround( m_oState, m_vGoal, m_eGroundGoal ) )
				break;

			m_eStage++;
			// Fallthrough
		case STAGE_Local:
			if ( m_pNavigator->BuildPathLocal( m_oState, m_fRadius, pFacing, m_eDoubleBuffer ) )
			{
				m_eResult = RESULT_Success;
				return TTRUE;
			}

			m_eStage++;
			// Fallthrough
		case STAGE_Network:
			if ( m_pNavigator->BuildPathNetwork( m_oState, m_fRadius, pFacing, m_eDoubleBuffer ) )
			{
				m_eResult = RESULT_Success;
				return TTRUE;
			}

			m_eStage++;
			// Fallthrough
		default:
			m_pNavigator->ClearGoal( TTRUE );
			break;
	}

	m_eResult = RESULT_Failed;
	return TTRUE;
}

// $deBlob: FUNCTION 007dd830
void ABuildPathJob::Begin()
{
	m_eStage = STAGE_CheckGround;
}

// $deBlob: FUNCTION 007dda20
ABuildPathJob::ABuildPathJob()
{
}

// $deBlob: FUNCTION 007ddae0
// $deBlob: FUNCTION 007dda50
ABuildPathJob::~ABuildPathJob()
{
}

// $deBlob: FUNCTION 007ddba0
void ABuildPathJob::Init( AAINavigator* a_pNavigator, const TVector4& a_rvStart, const TVector4& a_rvGoal, TFLOAT a_fRadius, TBOOL a_bUnk, const TVector4* a_pFacing, AAINavigator::eGroundGoal a_eGroundGoal, AAINavigator::eDoubleBuffer a_eDoubleBuffer )
{
	m_pNavigator    = a_pNavigator;
	m_oState.vStart = a_rvStart;
	m_vGoal         = a_rvGoal;
	m_bUnk28        = a_bUnk;
	m_eGroundGoal   = a_eGroundGoal;
	m_bHasFacing    = a_pFacing != TNULL;
	m_eDoubleBuffer = a_eDoubleBuffer;
	m_fRadius       = a_fRadius;

	if ( a_pFacing != TNULL )
		m_vFacing = *a_pFacing;
}
