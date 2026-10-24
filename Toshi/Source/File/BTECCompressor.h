#pragma once

TOSHI_NAMESPACE_START

class BTECCompressor
{
public:
	class Node
	{
	public:
		friend BTECCompressor;

	public:
		Node() = default;
		/*{
                m_pNext = this;
                m_pPrev = this;
            }*/

		void Reset()
		{
			m_pNext = this;
			m_pPrev = this;
		}

	private:
		Node*  m_pNext;
		Node*  m_pPrev;
		Node*  m_pHashNext;
		Node*  m_pHashPrev;
		TBYTE* m_pData;
		void*  m_Unk4;
	};

	using Bucket = Node**;

public:
	BTECCompressor();
	~BTECCompressor();

	void  Initialize( TBYTE* buffer, size_t bufferSize, TINT maxoffset, TINT unk );
	void  AdvanceWindow( size_t dataSize );
	TBOOL FindMatch( TBYTE* buffer, size_t bufferSize, TBYTE*& offset, size_t& dataSize );
	void  AllocSubstring( TBYTE* buffer );

	// The parts of AllocSubstring as functions
	// Moves the first free node to the used list (no free node: retail throws "SlidingWindow::AllocSubstring: No free
	// substrings!")
	Node* AllocNode();
	// Unlinks the node from its hash bucket and gives it back to the free list
	void FreeNode( Node* a_pNode );
	// Links the node into the bucket of the hash of its first a_uiSize bytes
	static void AddNodeToBucket( Node* a_pNode, Bucket a_pBucket, TINT a_iSize );

	static TBOOL FindNodeInBucket( TBYTE* buffer, Bucket nodeBucket, TINT bufferSize, Node*& out1, Node*& out2 );

	static TUINT32 HashData( TBYTE* data, TINT size )
	{
		TUINT32 hash = 0;
		if ( size > 0 )
		{
			do
			{
				hash = (TUINT32)*data ^ hash >> 0x1B ^ hash << 5;
				data += 1;
				size -= 1;
			} while ( size != 0 );
		}

		return hash;
	}

private:
	TBYTE*  m_Buffer1;
	TUINT32 m_MaxOffset;
	TBYTE*  m_DataEnd;
	TBYTE*  m_Buffer2;
	TBYTE*  m_Buffer3;
	Node*   m_pNodes;
	Node    m_Root1;
	Node    m_Root2;
	TBYTE*  m_Offsets[ 256 ];
	Bucket* m_Buckets;
	TINT    m_BucketCount;
};

TOSHI_NAMESPACE_END
