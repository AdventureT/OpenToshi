#pragma once
#include "AAINode.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2SList.h>

//-----------------------------------------------------------------------------
// Purpose: a 2D (x/z) hash of the AI network's nodes (PathFinding\AAINodeHash.cpp), created by AAINetwork::Load: 64x64
// buckets of node references (the grid of m_uiDivX x m_uiDivY cells wraps onto them) and the result buffer of the
// vicinity searches.
//-----------------------------------------------------------------------------
class AAINodeHash
    : public Toshi::TSingleton<AAINodeHash>
{
public:
	static TCONSTEXPR_VAR TINT BUCKETS_DIV  = 64;
	static TCONSTEXPR_VAR TINT MAX_RESULTS  = 500;
	static TCONSTEXPR_VAR TINT REFS_PER_ADD = 1024;

	struct HashRef
	    : public Toshi::T2SList<HashRef>::Node
	{
		AAINode* m_pNode; // 0x04
	};

	using HashBucket = Toshi::T2SList<HashRef>;

	// A node found by VicinityHashCompare and its squared distance
	struct NodeRef
	{
		AAINode* pNode;   // 0x00
		TFLOAT   fDistSq; // 0x04
	};

public:
	AAINodeHash( TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TUINT a_uiDivX, TUINT a_uiDivY );
	~AAINodeHash();

	void Hash( AAINode* a_pNode );
	void Release( AAINode* a_pNode );

	HashBucket* GetBucket( TFLOAT a_fX, TFLOAT a_fY );

	// The mean size of the non-empty buckets (debug)
	TFLOAT GetAverageBucketSize();

	// The nodes (with a flag of a_uiNodeMaskFlags) within a_fRange of a_rvPosition (at most a_uiMaxNodes, unsorted)
	NodeRef* VicinityHashCompare( const Toshi::TVector4& a_rvPosition, TFLOAT a_fRange, TUINT& a_ruiNumNodes, TUINT a_uiMaxNodes, TUINT8 a_uiNodeMaskFlags );

private:
	// Adds REFS_PER_ADD references to the free list
	void AddFreeRefs();

	// $deBlob: FUNCTION 007e1080
	TINT GetX( TFLOAT a_fX ) const
	{
		TASSERT( a_fX >= 0 );
		TFLOAT fX = m_fInvCellSizeX * a_fX;

		if ( fX < 0.0f )
			fX -= 1.0f;

		return TINT( fX );
	}

	// $deBlob: FUNCTION 007e10f0
	TINT GetY( TFLOAT a_fY ) const
	{
		TASSERT( a_fY >= 0 );
		TFLOAT fY = m_fInvCellSizeY * a_fY;

		if ( fY < 0.0f )
			fY -= 1.0f;

		return TINT( fY );
	}

	HashBucket& GetBucket( TINT a_iX, TINT a_iY ) { return m_aBuckets[ ( ( a_iX & ( BUCKETS_DIV - 1 ) ) << 6 ) + ( a_iY & ( BUCKETS_DIV - 1 ) ) ]; }

private:
	Toshi::T2SList<HashRef> m_oFreeRefs;                             // 0x0000
	HashBucket              m_aBuckets[ BUCKETS_DIV * BUCKETS_DIV ]; // 0x0004
	TFLOAT                  m_fOffsetX;                              // 0x4004
	TFLOAT                  m_fOffsetY;                              // 0x4008
	TUINT                   m_uiDivX;                                // 0x400C
	TUINT                   m_uiDivY;                                // 0x4010
	TFLOAT                  m_fInvCellSizeX;                         // 0x4014
	TFLOAT                  m_fInvCellSizeY;                         // 0x4018
	NodeRef                 m_aResults[ MAX_RESULTS ];               // 0x401C
};
