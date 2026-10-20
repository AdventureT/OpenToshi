#include "pch.h"
#include "A2DHashItem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

A2DHashItem::A2DHashItem()
    : m_uiStamp( 0 )
    , m_pHashRef( TNULL )
{
}

// $deBlob: FUNCTION 00763840
// $deBlob: FUNCTION 005ea110
A2DHashItem::~A2DHashItem()
{
	// The node's destructor asserts that the item was removed from its list
}
