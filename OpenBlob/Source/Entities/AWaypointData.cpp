#include "pch.h"
#include "AWaypointData.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AWaypointData ) == 0x4 );
TSTATICASSERT( sizeof( AWaypointData::Node ) == 0x20 );
TSTATICASSERT( sizeof( AWaypointData::Set ) == 0x8 );

// $deBlob: FUNCTION 00782ca0
AWaypointData::AWaypointData( TTRB* a_pTRB )
{
	m_pData = TNULL;
	m_pData = TSTATICCAST( Data, a_pTRB->GetSymbolAddress( "WaypointsMain" ) );
}

// $deBlob: FUNCTION 0059fe00
AWaypointData::Node* AWaypointData::Node::GetLinkedNode( TINT a_iLinkIndex, Set* a_pSet )
{
	TASSERT( a_iLinkIndex >= 0 && a_iLinkIndex < uiLinkCount );
	return &a_pSet->GetNode( pLinks[ a_iLinkIndex ].iNode );
}
