#pragma once
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

class APickup;

//-----------------------------------------------------------------------------
// Purpose: a pool of spawn points (Helpers\ASpawnPointPool.cpp): the positions of an entity's point list
// ("HazardPointList" of APropHazard / AContainerHazard), handed out and released again (the paint the blob loses on a
// hazard flies to one of them).
//-----------------------------------------------------------------------------
class ASpawnPointPool
{
public:
	struct SpawnPoint
	    : public Toshi::T2GenericDList::Node
	{
		// Leaves its list before the node's destructor asserts that it's unlinked
		// $deBlob: FUNCTION 007c0640
		// $deBlob: FUNCTION 007c0790
		~SpawnPoint() { Remove(); }

		Toshi::TVector4  m_vPosition; // 0x08
		ASpawnPointPool* m_pOwner;    // 0x18
	};

public:
	// a_rpPoints is the TVector4 array of the entity param (AEntityParams::GetArray)
	ASpawnPointPool( Toshi::TVector4* const& a_rpPoints, TUINT8 a_uiNumPoints );
	~ASpawnPointPool();

	// Releases every used point
	void ReleaseAll();

	// Takes the first free point (TNULL if none is free)
	SpawnPoint* GetSpawnPoint();

	// Takes the free point in the direction a_vDirection (normalised) from a_rPosition the most
	SpawnPoint* GetSpawnPoint( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_vDirection );

	void ReleaseSpawnPoint( SpawnPoint* a_pSpawnPoint );

	// A pickup callback (APickup 0x14C, user data 0x150 = the point): releases the point to its owner
	static void ReleaseSpawnPointCallback( APickup* a_pPickup, void* a_pSpawnPoint ); // (APickup::Callback)

	// $deBlob: FUNCTION 007b5b40
	TUINT8 GetNumFreePoints() const { return m_iFreePoints; }

private:
	SpawnPoint*                m_pPoints;     // 0x00
	Toshi::T2DList<SpawnPoint> m_lFreePoints; // 0x04
	Toshi::T2DList<SpawnPoint> m_lUsedPoints; // 0x0C
	TUINT8                     m_iNumPoints;  // 0x14
	TUINT8                     m_iFreePoints; // 0x15
};
