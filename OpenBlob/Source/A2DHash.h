#pragma once
#include "A2DHashItem.h"

#include <Core/TMemory.h>
#include <Math/TVector4.h>

struct A2DHashBucket;

//-----------------------------------------------------------------------------
// Purpose: a reference of an item in one bucket of an A2DHash. The references of an item are chained through m_pChain
// (the item's m_pHashRef is the head), the bucket and the free list link through m_pNext/m_pPrev.
//-----------------------------------------------------------------------------
struct A2DHashRef
{
	A2DHashItem*   m_pItem;   // 0x00
	A2DHashRef*    m_pChain;  // 0x04 the next reference of the same item
	A2DHashRef*    m_pNext;   // 0x08 link of the bucket or of the free list
	A2DHashRef*    m_pPrev;   // 0x0C
	A2DHashBucket* m_pBucket; // 0x10
};

//-----------------------------------------------------------------------------
// Purpose: a bucket of an A2DHash: the sentinel of the circular list of the references of its cell. The list links are
// at the same offsets as the ones of A2DHashRef, so a bucket can be the sentinel of a list of refs.
// NOTE: m_uiStamp is the generation of the last query that visited the bucket, the others are unknown
//-----------------------------------------------------------------------------
struct A2DHashBucket
{
	// The number of refs in the bucket's list (the free list's sentinel, an A2DHashRef, has the same layout)
	// $deBlob: FUNCTION 00569750
	TINT GetNumRefs() const
	{
		TINT iCount = 0;

		for ( A2DHashRef* pRef = m_pNext; pRef != (const A2DHashRef*)this; pRef = pRef->m_pNext )
			iCount++;

		return iCount;
	}

	TUINT32     m_uiUnk00; // 0x00
	TUINT32     m_uiUnk04; // 0x04
	A2DHashRef* m_pNext;   // 0x08
	A2DHashRef* m_pPrev;   // 0x0C
	TUINT32     m_uiUnk10; // 0x10
	TUINT32     m_uiStamp; // 0x14
};

//-----------------------------------------------------------------------------
// Purpose: a 2D spatial hash of the items (X and Z of their positions). The items are put in the cells of their
// bounding square (the cells are hashed into 1024 buckets by their coordinates) and queried with a sphere and a
// direction. The references come from a free list of a_uiNumRefs refs.
//-----------------------------------------------------------------------------
class A2DHash
{
public:
	typedef void ( *Callback )( TUINT32 );

	// A query result: an item and the value of the query (fRadius^2 + 2 * itemRadius * fRadius) (0x08 bytes)
	struct Result
	{
		A2DHashItem* m_pItem;     // 0x00
		TFLOAT       m_fDistance; // 0x04
	};

public:
	A2DHash();
	virtual ~A2DHash(); // +0x00

	// Sets up the hash over the area (a_fMinX, a_fMinY)-(a_fMaxX, a_fMaxY) with a_iXDiv by a_iYDiv cells; allocates the
	// results (a_uiNumResults), the refs (a_uiNumRefs) and the buckets (a_uiNumBuckets) from a_pHeap
	void Init( TMemoryHeap* a_pHeap, Callback a_pfnCallback, TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TINT a_iXDiv, TINT a_iYDiv, TUINT32 a_uiNumResults, TUINT32 a_uiNumRefs, TUINT32 a_uiNumBuckets );

	// Puts the item in the cells of its bounding square (it's removed from the hash first); returns the head of its refs
	A2DHashRef* Insert( A2DHashItem* a_pItem );

	// Removes the item from the cells and gives its refs back to the free list
	void Remove( A2DHashItem* a_pItem );

	// Queries the items within a_fRadius of a_rCenter that are in front of a_rDirection; the results are written up to
	// a_iMax of them (*a_piCount receives the number) and returned
	Result* Query( const Toshi::TVector4& a_rCenter, const Toshi::TVector4& a_rDirection, TFLOAT a_fRadius, TINT* a_piCount, TINT a_iMax );

	// The bounds of a hash over the level: the region 0 of the terrain (its first corner, 150 units per cell, with one
	// extra cell on each side) and its number of cells of a_fCellSize
	static void GetLevelBounds( TFLOAT a_fCellSize, TFLOAT* a_pfMinX, TFLOAT* a_pfMinY, TFLOAT* a_pfMaxX, TFLOAT* a_pfMaxY, TINT* a_piXDiv, TINT* a_piYDiv );
	// Debug
	// Draws the number of refs of each bucket as lines with the GUI renderer
	void DebugRender();
	// The cell of a coordinate (relative to the hash's offset)
	TINT GetCellX( TFLOAT a_fX ) const;
	TINT GetCellY( TFLOAT a_fY ) const;

private:
	// Adds the items of the bucket to the results; returns FALSE once the results are full
	TBOOL QueryBucket( A2DHashBucket* a_pBucket, const Toshi::TVector4& a_rCenter, const Toshi::TVector4& a_rDirection, TFLOAT a_fRadius, TINT* a_piCount, TINT a_iMax );

	// The cell of a coordinate
	static TINT GetCell( TFLOAT a_fValue );

public:
	static TUINT32 ms_uiGeneration; // the generation of the queries (the stamps of the items and buckets)

public:
	Callback       m_pfnCallback;    // 0x04 called with 0 once the generation wraps
	TUINT32        m_uiMaxResults;   // 0x08
	TUINT32        m_uiNumRefs;      // 0x0C
	TUINT32        m_uiNumBuckets;   // 0x10
	TFLOAT         m_fXMax;          // 0x14
	TFLOAT         m_fYMax;          // 0x18
	TINT           m_iXDiv;          // 0x1C
	TINT           m_iYDiv;          // 0x20
	TFLOAT         m_fXOffset;       // 0x24 the offset of the cells to the non negative coordinates
	TFLOAT         m_fYOffset;       // 0x28
	TFLOAT         m_fCellWidth;     // 0x2C
	TFLOAT         m_fCellHeight;    // 0x30
	TFLOAT         m_fInvCellWidth;  // 0x34
	TFLOAT         m_fInvCellHeight; // 0x38
	A2DHashBucket* m_paBuckets;      // 0x3C
	Result*        m_paResults;      // 0x40
	A2DHashRef     m_FreeListRoot;   // 0x44 the sentinel of the free refs
	A2DHashRef*    m_paRefs;         // 0x58
};
