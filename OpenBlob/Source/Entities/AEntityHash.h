#pragma once
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>
#include "AEntityHashFilter.h"

class ABaseEntity;
class AEntityFactory;

//-----------------------------------------------------------------------------
// Purpose: one link of an entity into a bucket of AEntityHash (AEntityHash.h); an entity's refs are chained through
// m_pNextRef (ABaseEntity::GetHashRef). The buckets' list roots and the free list root are refs as well.
//-----------------------------------------------------------------------------
struct AEntityHashRef
{
	AEntityHashRef()
	    : m_pEntity( TNULL ), m_pNextRef( TNULL )
	{
		m_Prev    = this;
		m_Next    = this;
		m_pBucket  = TNULL;
	}

	// $deBlob: FUNCTION 007742e0
	TBOOL IsLinked() const { return m_Next != this && m_Prev != this; }

	// $deBlob: FUNCTION 00774320
	void Unlink()
	{
		TASSERT( IsLinked() );
		m_Next->m_Prev = m_Prev;
		m_Prev->m_Next = m_Next;
		m_Next         = this;
		m_Prev         = this;
	}

	// $deBlob: FUNCTION 00774280
	void LinkAfter( AEntityHashRef* a_pRoot )
	{
		TASSERT( !IsLinked() );
		m_Prev                  = a_pRoot;
		m_Next                  = a_pRoot->m_Next;
		a_pRoot->m_Next->m_Prev = this;
		m_Prev->m_Next          = this;
	}

	ABaseEntity*    m_pEntity;  // 0x00
	AEntityHashRef* m_pNextRef; // 0x04 the entity's next ref
	AEntityHashRef* m_Next;     // 0x08
	AEntityHashRef* m_Prev;     // 0x0C
	void*           m_pBucket;  // 0x10 AEntityHash::Bucket
};

//-----------------------------------------------------------------------------
// Purpose: a spatial hash of the entities on the XZ plane: 512 buckets and a pool of 7168 refs; each entity is linked
// into the buckets its bounding sphere covers.
//-----------------------------------------------------------------------------
class AEntityHash
    : public Toshi::TSingleton<AEntityHash>
{
public:
	static TCONSTEXPR_VAR TINT AUNITHASH_BUCKET_COUNT = 512;
	static TCONSTEXPR_VAR TINT MAX_REFS               = 7168;

	struct Bucket
	{
		// $deBlob: FUNCTION 00774080
		Bucket()
		    : m_iTimeStamp( 0 )
		{}

		AEntityHashRef m_oEntities;  // 0x00
		AEntityHashRef m_oPickups;   // 0x14 entities with ABaseEntity::FLAG_Pickup
		TINT           m_iTimeStamp; // 0x28
	};

public:
	AEntityHash();
	// $deBlob: FUNCTION 00774120
	// $deBlob: FUNCTION 00773200
	virtual ~AEntityHash() = default;

	void Init( AEntityFactory* a_pFactory, TFLOAT a_fXMin, TFLOAT a_fYMin, TFLOAT a_fXMax, TFLOAT a_fYMax, TINT a_iXDiv, TINT a_iYDiv );

	AEntityHashRef* Add( ABaseEntity& a_roUnit, TBOOL a_bUnk );
	void            Remove( ABaseEntity& a_roUnit );

	// Returns a new time stamp for the hash queries (on wrap-around the entities' stamps are reset)
	TUINT32 NextTimeStamp();

	// Entities within a_fRadius of a_rvPos (the bounding spheres overlap, the distance in XZ unless a_bUse3D),
	// pickups first; returns the array of the found entities (MAX_QUERY_RESULTS) and their count in *a_piCount.
	// The count stops at a_iMax. a_bBruteForce is deprecated, a_bIgnorePickups and a_bIgnoreEverythingElse can't both
	// be set.
	ABaseEntity** Query( const Toshi::TVector4& a_rvPos, TFLOAT a_fRadius, TINT* a_piCount, AEntityHashFilter* a_pFilter, TBOOL a_bUse3D, TINT a_iMax, TBOOL a_bBruteForce, TBOOL a_bIgnorePickups, TBOOL a_bIgnoreEverythingElse );

	// Debug
	// Writes the entity and pickup counts of each bucket to a_szFileName ("total,entities,pickups")
	void DumpBucketCounts( const TCHAR* a_szFileName );
	// Draws the bucket counts as lines with the GUI renderer
	void DebugRenderBucketCounts();
	// The cell of a coordinate (relative to the hash's offset)
	TINT GetCellX( TFLOAT a_fX ) const;
	TINT GetCellY( TFLOAT a_fY ) const;

private:
	// Adds the entities of a_pRoot's list in range to the query's results; FALSE when the results are full
	TBOOL QueryList( AEntityHashRef* a_pRoot, const Toshi::TVector4& a_rvPos, TFLOAT a_fRadius, TBOOL a_bUse3D, TINT* a_piCount, TINT a_iMax, AEntityHashFilter* a_pFilter );

public:
	static TCONSTEXPR_VAR TINT MAX_QUERY_RESULTS = 500;

private:
	static TUINT32 ms_uiTimeStamp;

private:
	AEntityFactory* m_pFactory;                            // 0x04
	TFLOAT          m_fXMax;                               // 0x08
	TFLOAT          m_fYMax;                               // 0x0C
	TINT            m_iXDiv;                               // 0x10
	TINT            m_iYDiv;                               // 0x14
	TFLOAT          m_fXOffset;                            // 0x18
	TFLOAT          m_fYOffset;                            // 0x1C
	TFLOAT          m_fCellWidth;                          // 0x20
	TFLOAT          m_fCellHeight;                         // 0x24
	TFLOAT          m_fInvCellWidth;                       // 0x28
	TFLOAT          m_fInvCellHeight;                      // 0x2C
	Bucket          m_aBuckets[ AUNITHASH_BUCKET_COUNT ];  // 0x30
	ABaseEntity*    m_apQueryResults[ MAX_QUERY_RESULTS ]; // 0x5830 the results of Query
	AEntityHashRef  m_FreeListRoot;                        // 0x6000
	AEntityHashRef  m_aRefs[ MAX_REFS ];                   // 0x6014
};
