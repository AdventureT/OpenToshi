#include "pch.h"
#include "AAINode.h"
#include "AAINodeHash.h"

#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AAILink ) == 0x10 );
TSTATICASSERT( sizeof( AAINode ) == 0x1C );

// $deBlob: FUNCTION 007dfea0
void AAINode::AddLink( AAILink* a_pLink )
{
	for ( TINT i = 0; i < m_uiLinksCount; i++ )
	{
		TASSERT( a_pLink != m_apLinks[ i ], "Adding the same link twice?" );

		if ( m_apLinks[ i ] == TNULL )
		{
			m_apLinks[ i ] = a_pLink;
			return;
		}
	}
}

// $deBlob: FUNCTION 007dff20
AAILink* AAINode::GetLinkTo( AAINode* a_pNode )
{
	for ( TINT i = 0; i < m_uiLinksCount; i++ )
	{
		TASSERT( TNULL != m_apLinks[ i ] );

		if ( a_pNode == m_apLinks[ i ]->GetOtherNode( this ) )
			return m_apLinks[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 007dffb0
AAINode::AAINode()
{
	m_vPosition    = TVector4::VEC_ZERO;
	m_apLinks      = TNULL;
	m_pAStarNode   = TNULL;
	m_uiFlags      = FLAG_Unk1;
	m_uiSegmentID  = 0;
	m_uiLinksCount = 0;
}

// $deBlob: FUNCTION 007dffe0
// $deBlob: FUNCTION 0078f7f0
AAINode::~AAINode()
{
	AAINodeHash::GetSingleton()->Release( this );
	delete[] m_apLinks;
}

// $deBlob: FUNCTION 007e0000
void AAINode::Initialise( TUINT8 a_uiLinksCount )
{
	m_vPosition  = TVector4::VEC_ZERO;
	m_uiFlags    = FLAG_Unk1;
	m_pAStarNode = TNULL;
	Create( a_uiLinksCount );
	m_uiSegmentID = 0;
}

// $deBlob: FUNCTION 007e0030
void AAINode::Create( TUINT8 a_uiLinksCount )
{
	TASSERT( 0 == m_uiLinksCount );
	m_uiLinksCount = a_uiLinksCount;

	if ( a_uiLinksCount != 0 )
	{
		TASSERT( TNULL == m_apLinks );
		m_apLinks = new AAILink*[ m_uiLinksCount ];

		for ( TINT i = 0; i < m_uiLinksCount; i++ )
			m_apLinks[ i ] = TNULL;
	}
}

// $deBlob: FUNCTION 007e0120
void AAILink::SetStale( TFLOAT a_fTime )
{
	if ( a_fTime <= 0.0f )
	{
		m_uiFlags &= ~FLAG_Stale;
	}
	else
	{
		m_uiFlags |= FLAG_Stale;
		m_fStaleTime = TSystemManager::GetSingleton()->GetScheduler()->GetTotalTime() + a_fTime;
	}
}

// $deBlob: FUNCTION 007e0160
TBOOL AAILink::IsStaleImp()
{
	TASSERT( IsFlagged( FLAG_Stale ) );

	if ( TSystemManager::GetSingleton()->GetScheduler()->GetTotalTime() <= m_fStaleTime )
		return TTRUE;

	m_uiFlags &= ~FLAG_Stale;
	return TFALSE;
}
