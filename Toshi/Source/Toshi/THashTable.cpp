#include "ToshiPCH.h"
#include "THashTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006efd20
TUINT32 THashTable::DefaultItemHashFunc( void* a_pMem, TINT a_iSize, TINT a_iMaxValue )
{
	const TBYTE* pBuffer = TSTATICCAST( const TBYTE, a_pMem );
	TUINT32      uiHash  = 0;

	for ( TINT i = 0; i < a_iSize; i++ )
	{
		uiHash = ( uiHash << 4 ) + pBuffer[ i ];

		if ( ( uiHash & 0xF0000000 ) != 0 )
			uiHash ^= ( uiHash & 0xF0000000 ) ^ ( ( uiHash & 0xF0000000 ) >> 24 );
	}

	return uiHash % TUINT32( a_iMaxValue );
}

// $deBlob: FUNCTION 006efd70
TBOOL THashTable::DefaultItemCompareFunc( void* a_pMem1, void* a_pMem2, TINT a_iSize )
{
	return TUtil::MemCompare( a_pMem1, a_pMem2, a_iSize ) ? TFALSE : TTRUE;
}

// $deBlob: FUNCTION 006efd90
TBOOL THashTable::DefaultItemIterateFunc( void* a_pItem )
{
	return TTRUE;
}

// $deBlob: FUNCTION 006ef810
THashTable::THashTable()
{
	m_iBucketSize         = 0;
	m_iItemSize           = 0;
	m_iItemCount          = 0;
	m_iItemCountTotal     = 0;
	m_iHashNodeCountTotal      = 0;
	m_iHashNodeCount = 0;
	m_pBuckets            = TNULL;
	m_pHashNodes          = TNULL;
	m_pItems              = TNULL;
	m_ItemHashFunc        = TNULL;
	m_ItemCompareFunc     = TNULL;
}

// $deBlob: FUNCTION 006ef860
THashTable::~THashTable()
{
	// NOTE: doesn't destroy the table, the owner calls Destroy
	TASSERT( TNULL == m_pBuckets );
}

// $deBlob: FUNCTION 006ef8a0
TBOOL THashTable::Create( TINT a_iItemCountTotal, TINT a_iItemSize, TINT a_iBucketSize, TINT a_iHashNodeCount )
{
	TASSERT( TNULL == m_pBuckets );

	if ( m_pBuckets != TNULL )
		return TFALSE;

	m_iItemSize       = a_iItemSize;
	m_iItemCountTotal = a_iItemCountTotal;
	m_iBucketSize     = a_iBucketSize;
	m_iItemCount      = 0;
	m_iHashNodeCountTotal  = ( a_iItemCountTotal > a_iHashNodeCount ) ? a_iItemCountTotal : a_iHashNodeCount;
	m_pBuckets        = new TINT[ m_iBucketSize ];
	m_pHashNodes      = new HashNode[ m_iHashNodeCountTotal ];
	m_pItems          = new TBYTE[ m_iItemSize * m_iItemCountTotal ];
	m_ItemHashFunc    = DefaultItemHashFunc;
	m_ItemCompareFunc = DefaultItemCompareFunc;

	for ( TINT i = 0; i < m_iBucketSize; i++ )
		m_pBuckets[ i ] = INVALID_BUCKET_ID;

	return TTRUE;
}

// $deBlob: FUNCTION 006ef990
void THashTable::Destroy()
{
	delete[] m_pBuckets;
	m_pBuckets = TNULL;

	delete[] m_pHashNodes;
	m_pHashNodes = TNULL;

	// NOTE: like retail, m_pItems isn't reset
	delete[] m_pItems;
}

// $deBlob: FUNCTION 006efd00
THashTable::t_ItemHashFunc THashTable::SetItemHashFunction( t_ItemHashFunc a_fnHash )
{
	t_ItemHashFunc fnOld = m_ItemHashFunc;
	m_ItemHashFunc       = a_fnHash;
	return fnOld;
}

// $deBlob: FUNCTION 006efd10
THashTable::t_ItemCompareFunc THashTable::SetItemCompareFunction( t_ItemCompareFunc a_fnCompare )
{
	t_ItemCompareFunc fnOld = m_ItemCompareFunc;
	m_ItemCompareFunc       = a_fnCompare;
	return fnOld;
}

// $deBlob: FUNCTION 006ef9c0
THashTable::HashNode* THashTable::Find( void* a_pData )
{
	TASSERT( 0 != m_iItemSize );

	if ( m_pBuckets == TNULL )
		return TNULL;

	TINT iNode = m_pBuckets[ m_ItemHashFunc( a_pData, m_iItemSize, m_iBucketSize ) ];

	while ( iNode != INVALID_BUCKET_ID )
	{
		HashNode* pNode = &m_pHashNodes[ iNode ];

		if ( m_ItemCompareFunc( a_pData, m_pItems + m_iItemSize * pNode->iItemIndex, m_iItemSize ) == TTRUE )
			return pNode;

		iNode = pNode->iNext;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006efad0
THashTable::HashNode* THashTable::Append( void* a_pData )
{
	TASSERT( 0 != m_iItemSize );

	if ( m_pBuckets == TNULL )
		return TNULL;

	TASSERT( !( ( m_iHashNodeCount == m_iHashNodeCountTotal ) || ( m_iItemCount == m_iItemCountTotal ) ) );

	if ( ( m_iHashNodeCount == m_iHashNodeCountTotal ) || ( m_iItemCount == m_iItemCountTotal ) )
		return TNULL;

	TUINT32   uiBucket = m_ItemHashFunc( a_pData, m_iItemSize, m_iBucketSize );
	HashNode* pNode    = &m_pHashNodes[ m_iHashNodeCount ];

	pNode->iNext      = m_pBuckets[ uiBucket ];
	pNode->iItemIndex = m_iItemCount;
	TUtil::MemCopy( m_pItems + m_iItemSize * m_iItemCount, a_pData, m_iItemSize );

	m_pBuckets[ uiBucket ] = m_iHashNodeCount++;
	m_iItemCount++;

	return pNode;
}

// $deBlob: FUNCTION 006efa60
THashTable::HashNode* THashTable::Insert( void* a_pData )
{
	TASSERT( 0 != m_iItemSize );

	if ( m_pBuckets == TNULL )
		return TNULL;

	HashNode* pNode = Find( a_pData );
	return pNode ? pNode : Append( a_pData );
}

// $deBlob: FUNCTION 006efbe0
TBOOL THashTable::Iterate( t_ItemIterateFunc a_fnIterate )
{
	TASSERT( 0 != m_iItemSize );

	TBYTE* pItem = m_pItems;

	for ( TINT i = 0; i < m_iItemCount; i++ )
	{
		if ( !a_fnIterate( pItem ) )
			return TFALSE;

		pItem += m_iItemSize;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006efc50
void* THashTable::GetItemArray()
{
	return m_pItems;
}

// $deBlob: FUNCTION 006efc60
TINT THashTable::GetItemSize()
{
	return m_iItemSize;
}

// $deBlob: FUNCTION 006efc70
TINT THashTable::GetItemCount()
{
	return m_iItemCount;
}

// $deBlob: FUNCTION 006efc80
TINT THashTable::GetItemCountTotal()
{
	return m_iItemCountTotal;
}

// $deBlob: FUNCTION 006efc90
TINT* THashTable::GetBuckets()
{
	return m_pBuckets;
}

// $deBlob: FUNCTION 006efca0
void THashTable::DeleteBucketMemory()
{
	delete[] m_pBuckets;
	m_pBuckets = TNULL;
}

// $deBlob: FUNCTION 006efcc0
void THashTable::DeleteItemMemory()
{
	delete[] m_pItems;
}

// $deBlob: FUNCTION 006efcd0
void THashTable::SetItemMemory( void* a_pItems )
{
	// NOTE: like retail, frees the buckets (not the old items)
	delete[] m_pBuckets;
	m_pBuckets = TNULL;
	m_pItems   = TSTATICCAST( TBYTE, a_pItems );
}

TOSHI_NAMESPACE_END
