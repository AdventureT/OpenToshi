#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a fixed size hash table of items of m_iItemSize bytes (Create allocates everything once).
// NOTE: names follow de Blob's asserts: m_pBuckets is the hash -> first node array, m_pHashNodes the node chain.
//-----------------------------------------------------------------------------
class THashTable
{
public:
	struct HashNode
	{
		TINT iNext;      // 0x0 next node of the bucket, INVALID_BUCKET_ID ends the chain
		TINT iItemIndex; // 0x4
	};

	inline static constexpr TINT INVALID_BUCKET_ID = -1;

	using t_ItemCompareFunc = TBOOL ( * )( void* a_pMem1, void* a_pMem2, TINT a_iSize );
	using t_ItemHashFunc    = TUINT32 ( * )( void* a_pMem, TINT a_iSize, TINT a_iMaxValue );
	using t_ItemIterateFunc = TBOOL ( * )( void* a_pItem );

public:
	THashTable();
	~THashTable();

	TBOOL Create( TINT a_iItemCountTotal, TINT a_iItemSize, TINT a_iBucketSize, TINT a_iHashNodeCount );
	void  Destroy();

	t_ItemHashFunc    SetItemHashFunction( t_ItemHashFunc a_fnHash );
	t_ItemCompareFunc SetItemCompareFunction( t_ItemCompareFunc a_fnCompare );

	HashNode* Find( void* a_pData );
	HashNode* Append( void* a_pData );
	HashNode* Insert( void* a_pData );

	// Calls a_fnIterate for every item until it returns TFALSE. Returns TFALSE if it was stopped
	TBOOL Iterate( t_ItemIterateFunc a_fnIterate );

	void* GetItemArray();
	TINT  GetItemSize();
	TINT  GetItemCount();
	TINT  GetItemCountTotal();
	TINT* GetBuckets();

	void DeleteBucketMemory();
	void DeleteItemMemory();
	void SetItemMemory( void* a_pItems );

public:
	static TUINT32 DefaultItemHashFunc( void* a_pMem, TINT a_iSize, TINT a_iMaxValue );
	static TBOOL   DefaultItemCompareFunc( void* a_pMem1, void* a_pMem2, TINT a_iSize );
	static TBOOL   DefaultItemIterateFunc( void* a_pItem );

private:
	TINT              m_iBucketSize;         // 0x00
	TINT              m_iItemSize;           // 0x04
	TINT              m_iItemCount;          // 0x08
	TINT              m_iItemCountTotal;     // 0x0C
	TINT              m_iHashNodeCount;      // 0x10
	TINT              m_iHashNodeCountTotal; // 0x14 number of used nodes
	TINT*             m_pBuckets;            // 0x18
	HashNode*         m_pHashNodes;          // 0x1C
	TBYTE*            m_pItems;              // 0x20
	t_ItemHashFunc    m_ItemHashFunc;        // 0x24
	t_ItemCompareFunc m_ItemCompareFunc;     // 0x28
}; // 0x2C

TOSHI_NAMESPACE_END
