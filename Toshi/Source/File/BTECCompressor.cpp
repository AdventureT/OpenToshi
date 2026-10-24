#include "ToshiPCH.h"
#include "BTECCompressor.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0068ac10
BTECCompressor::BTECCompressor()
{
	this->m_Buffer1     = TNULL;
	this->m_DataEnd     = TNULL;
	this->m_Buffer2     = TNULL;
	this->m_Buffer3     = TNULL;
	this->m_pNodes      = TNULL;
	this->m_Buckets     = TNULL;
	this->m_BucketCount = 0;
}

// $deBlob: FUNCTION 0068ada0
// $deBlob: FUNCTION 0068ac50
BTECCompressor::~BTECCompressor()
{
	m_Buffer1 = TNULL;
	m_DataEnd = TNULL;
	m_Buffer2 = TNULL;
	m_Buffer3 = TNULL;
	delete[] m_pNodes;

	for ( TINT i = 0; i < m_BucketCount; i++ )
	{
		for ( TINT k = 0; k < 256; k++ )
		{
			delete m_Buckets[ i ][ k ];
		}

		delete[] m_Buckets[ i ];
	}

	delete[] m_Buckets;
	m_Buckets     = TNULL;
	m_BucketCount = 0;
}

// $deBlob: FUNCTION 0068ac60
void BTECCompressor::Initialize( TBYTE* buffer, TSIZE bufferSize, TINT maxoffset, TINT unk )
{
	m_DataEnd     = buffer + bufferSize;
	m_Buffer1     = buffer;
	m_Buffer2     = buffer;
	m_Buffer3     = buffer;
	m_MaxOffset   = maxoffset;
	m_BucketCount = ( unk < 1 ) ? 0 : unk - 1;

	m_Root1.m_pNext = &m_Root1;
	m_Root1.m_pPrev = &m_Root1;
	m_Root2.m_pNext = &m_Root2;
	m_Root2.m_pPrev = &m_Root2;

	m_pNodes = new Node[ m_BucketCount * maxoffset ];

	for ( TINT i = 0; i < m_BucketCount * maxoffset; i++ )
	{
		m_pNodes[ i ].m_pNext          = &m_Root1;
		m_pNodes[ i ].m_pPrev          = m_Root1.m_pPrev;
		m_pNodes[ i ].m_pNext->m_pPrev = &m_pNodes[ i ];
		m_pNodes[ i ].m_pPrev->m_pNext = &m_pNodes[ i ];
	}

	TUtil::MemClear( m_Offsets, sizeof( m_Offsets ) );

	m_Buckets = new Bucket[ m_BucketCount ];

	for ( TINT i = 0; i < m_BucketCount; i++ )
	{
		m_Buckets[ i ] = new Node*[ 256 ];

		for ( TINT k = 0; k < 256; k++ )
		{
			Node* someNode        = new Node;
			someNode->m_pHashNext = someNode;
			someNode->m_pHashPrev = someNode;
			m_Buckets[ i ][ k ]   = someNode;
		}
	}
}

// $deBlob: FUNCTION 0068ae40
void BTECCompressor::AdvanceWindow( TSIZE dataSize )
{
	auto oldBuffer3 = m_Buffer3;
	auto newBuffer3 = m_Buffer3 + dataSize;

	if ( m_DataEnd <= newBuffer3 && newBuffer3 != m_DataEnd )
	{
		newBuffer3 = m_DataEnd;
	}

	m_Buffer3 = newBuffer3;

	if ( m_MaxOffset < static_cast<uintptr_t>( m_Buffer3 - m_Buffer2 ) )
	{
		m_Buffer2 = m_Buffer3 - m_MaxOffset;

		for ( TSIZE i = 0; i < 256; i++ )
		{
			if ( m_Offsets[ i ] < m_Buffer2 )
			{
				m_Offsets[ i ] = TNULL;
			}
		}

		auto node = m_Root2.m_pNext;
		while ( node != &m_Root2 )
		{
			if ( m_Buffer2 <= node->m_pData ) break;
			auto nextNode = node->m_pNext;

			node->m_pHashNext->m_pHashPrev = node->m_pHashPrev;
			node->m_pHashPrev->m_pHashNext = node->m_pHashNext;
			node->m_pHashNext              = node;
			node->m_pHashPrev              = node;
			node->m_pNext->m_pPrev         = node->m_pPrev;
			node->m_pPrev->m_pNext         = node->m_pNext;
			node->m_pNext                  = &m_Root1;
			node->m_pPrev                  = m_Root1.m_pPrev;
			m_Root1.m_pPrev                = node;
			node->m_pPrev->m_pNext         = node;

			node = nextNode;
		}
	}

	while ( oldBuffer3 < m_Buffer3 )
	{
		AllocSubstring( oldBuffer3 );
		oldBuffer3++;
	}
}

// $deBlob: FUNCTION 0068af10
TBOOL BTECCompressor::FindMatch( TBYTE* buffer, TSIZE bufferSize, TBYTE*& offset, TSIZE& dataSize )
{
	offset   = TNULL;
	dataSize = 0;

	if ( bufferSize < 1 || m_Offsets[ (TBYTE)*buffer ] == TNULL )
	{
		return TFALSE;
	}

	if ( m_BucketCount > 0 )
	{
		TSIZE  size   = 0;
		Node*  node1  = TNULL;
		Node*  node2  = TNULL;
		Bucket bucket = TNULL;

		for ( TSIZE i = 0; i < m_BucketCount; i++ )
		{
			TSIZE localSize = 2 << ( i % 32 );
			if ( bufferSize < localSize ) break;

			Node* localNode1;
			Node* localNode2;
			TBOOL bRes = FindNodeInBucket( buffer, m_Buckets[ i ], localSize, localNode1, localNode2 );
			if ( bRes == TFALSE ) break;

			size   = localSize;
			node1  = localNode1;
			node2  = localNode2;
			bucket = m_Buckets[ i ];
		}

		if ( node1 != TNULL )
		{
			TSIZE someSize;

			if ( bucket == m_Buckets[ m_BucketCount - 1 ] )
			{
				someSize = TMath::Min( bufferSize, static_cast<TSIZE>( m_DataEnd - node1->m_pData ) );
			}
			else
			{
				someSize = TMath::Min( bufferSize, size * 2 - 1 );
			}

			if ( node1 == node2 )
			{
				return TTRUE;
			}

			while ( TTRUE )
			{
				size = 0;

				while ( node1->m_pData[ size ] == buffer[ size ] )
				{
					if ( someSize <= size ) break;
					size++;
				}

				if ( dataSize < size )
				{
					dataSize = size;
					offset   = node1->m_pData;

					if ( someSize <= size )
					{
						return TTRUE;
					}
				}

				node1 = node1->m_pHashNext;

				if ( node1 == node2 )
				{
					return TTRUE;
				}
			}
		}
	}

	offset   = m_Offsets[ (TBYTE)*buffer ];
	dataSize = 1;

	return TTRUE;
}

// $deBlob: FUNCTION 0068b180
void BTECCompressor::AllocSubstring( TBYTE* buffer )
{
	m_Offsets[ (TBYTE)*buffer ] = buffer;

	for ( TSIZE i = 0; i < m_BucketCount; i++ )
	{
		TSIZE uiSubstringSize = 2 << ( i % 32 );

		if ( static_cast<uintptr_t>( m_DataEnd - buffer ) < uiSubstringSize )
		{
			return;
		}

		auto node = m_Root1.m_pNext;
		if ( node == &m_Root1 )
		{
			throw "SlidingWindow::AllocSubstring: No free substrings!";
		}

		node->m_pNext->m_pPrev = node->m_pPrev;
		node->m_pPrev->m_pNext = node->m_pNext;
		node->m_pNext          = &m_Root2;
		node->m_pPrev          = m_Root2.m_pPrev;
		m_Root2.m_pPrev        = node;
		node->m_pPrev->m_pNext = node;
		node->m_Unk4           = (void*)( m_DataEnd - buffer );
		node->m_pData          = buffer;

		TUINT32 hash = HashData( buffer, uiSubstringSize );

		auto hashedNode                = m_Buckets[ i ][ hash % 256 ];
		node->m_pHashNext              = hashedNode;
		node->m_pHashPrev              = hashedNode->m_pHashPrev;
		hashedNode->m_pHashPrev        = node;
		node->m_pHashPrev->m_pHashNext = node;
	}
}

// $deBlob: FUNCTION 0068b0a0
BTECCompressor::Node* BTECCompressor::AllocNode()
{
	Node* node = m_Root1.m_pNext;

	if ( node == &m_Root1 )
	{
		throw "SlidingWindow::AllocSubstring: No free substrings!";
	}

	node->m_pNext->m_pPrev = node->m_pPrev;
	node->m_pPrev->m_pNext = node->m_pNext;
	node->m_pNext          = &m_Root2;
	node->m_pPrev          = m_Root2.m_pPrev;
	m_Root2.m_pPrev        = node;
	node->m_pPrev->m_pNext = node;

	return node;
}

// $deBlob: FUNCTION 0068b0f0
void BTECCompressor::FreeNode( Node* a_pNode )
{
	a_pNode->m_pHashNext->m_pHashPrev = a_pNode->m_pHashPrev;
	a_pNode->m_pHashPrev->m_pHashNext = a_pNode->m_pHashNext;
	a_pNode->m_pHashNext              = a_pNode;
	a_pNode->m_pHashPrev              = a_pNode;

	a_pNode->m_pNext->m_pPrev = a_pNode->m_pPrev;
	a_pNode->m_pPrev->m_pNext = a_pNode->m_pNext;
	a_pNode->m_pNext          = &m_Root1;
	a_pNode->m_pPrev          = m_Root1.m_pPrev;
	m_Root1.m_pPrev           = a_pNode;
	a_pNode->m_pPrev->m_pNext = a_pNode;
}

// $deBlob: FUNCTION 0068b280
void BTECCompressor::AddNodeToBucket( Node* a_pNode, Bucket a_pBucket, TINT a_iSize )
{
	TUINT32 hash = HashData( a_pNode->m_pData, a_iSize );

	Node* hashedNode                  = a_pBucket[ hash % 256 ];
	a_pNode->m_pHashNext              = hashedNode;
	a_pNode->m_pHashPrev              = hashedNode->m_pHashPrev;
	hashedNode->m_pHashPrev           = a_pNode;
	a_pNode->m_pHashPrev->m_pHashNext = a_pNode;
}

// $deBlob: FUNCTION 0068b300
TBOOL BTECCompressor::FindNodeInBucket( TBYTE* buffer, Bucket nodeBucket, TINT bufferSize, Node*& out1, Node*& out2 )
{
	TUINT32 hash = HashData( buffer, bufferSize );

	out2 = nodeBucket[ hash % 256 ];
	out1 = out2->m_pHashNext;

	while ( out1 != out2 )
	{
		// NOTE: de Blob compares with T2String8::Compare (stops at a 0 byte), FindMatch verifies the match anyway
		TINT difference = T2String8::Compare( (const TCHAR*)out1->m_pData, (const TCHAR*)buffer, bufferSize );
		if ( difference == 0 ) break;

		out1 = out1->m_pHashNext;
	}

	return out1 != out2;
}

TOSHI_NAMESPACE_END
