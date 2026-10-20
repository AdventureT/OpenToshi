#pragma once
#include "ACivWrapper.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>

class A2DHash;

//-----------------------------------------------------------------------------
// Purpose: the civilians spawned in the world: their wrappers (ACivWrapper, an array of m_iNumCivs) in four lists, and
// the A2DHash of their positions. The lists 0 to 2 hold the wrappers in use, list 3 the free ones.
//-----------------------------------------------------------------------------
class ASpawnedCivManager
    : public Toshi::TSingleton<ASpawnedCivManager>
{
public:
	ASpawnedCivManager();
	~ASpawnedCivManager();

	// Allocates the wrappers (m_iNumCivs of them, all free) and the hash of the terrain's region; TRUE on success
	TBOOL Create();

	// Takes a free wrapper, puts it at a_rPosition facing a_rFacing and in the hash; with an ACiv from the pool when
	// one is free (otherwise the wrapper is only a point of radius 2). Returns TNULL when no wrapper is free
	ACivWrapper* AcquireWrapper( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rFacing, TINT a_iParam );

	// Takes a wrapper out of the hash and gives its ACiv back to the pool, then puts it back in the free list
	TBOOL ReleaseWrapper( ACivWrapper* a_pWrapper );

	// Gives every wrapper in use back to the free list
	void ReleaseAll();

	// The frame of the civilians: the wrappers in the view of a used camera are kept in list 0, the ones that left it
	// give their ACiv back to the pool, the ones in view get one from the pool (up to 5 a frame) and are started from
	// their respawn data (called every frame by AInGameState::OnUpdate)
	void Update( TFLOAT a_fDeltaTime );

public:
	// The stamp of every wrapper in use is set to the generation (the callback of the hash, called with 0 on wrap)
	static void SetWrapperStamps( TUINT32 a_uiGeneration );

public:
	Toshi::T2GenericDList::Node m_aCivLists[ 4 ]; // 0x00 the sentinels of the four lists (0 to 2: in use, 3: free)
	ACivWrapper*                m_paCivWrappers;  // 0x20
	TINT                        m_iNumWrappers;   // 0x24
	TINT                        m_iNumCivs;       // 0x28 the number of civilians (the pool holds up to 64 of them)
	A2DHash*                    m_pHash;          // 0x2C
};
