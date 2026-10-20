#pragma once
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

namespace Toshi
{
class TTRB;
class TParticleEffect;
} // namespace Toshi

class AEnvEmitter;

//-----------------------------------------------------------------------------
// Purpose: the level's Z-jump sets/points/nodes (AZJumpData.cpp, AZJumpData.h per the asserts), the "ZJumpData" symbol
// of the level's ZJumpData block.
// NOTE: only the loader is ported so far; the layout of the data isn't
//-----------------------------------------------------------------------------
class AZJumpData
    : public Toshi::TSingleton<AZJumpData>
{
public:
	enum NodeType : TINT
	{
		NodeType_Instance = 1, // the node's owner is a terrain cell instance (the assert's TYPE_INSTANCE)
		NodeType_Entity   = 2, // the node's owner is an entity
	};

	struct Set;
	struct Point;

	// A node of a set (0x18 bytes)
	struct Node
	{
		// The position of the node's owner: the translation of an instance's matrix, an entity's position, else
		// X0Y0Z0W1.
		const Toshi::TVector4& GetPosition() const;

		TINT   m_eType;        // 0x00 NodeType
		TUINT8 m_auUnk04[ 4 ]; // 0x04
		Set*   m_pSet;         // 0x08 the set of the node
		void*  m_pInstance;    // 0x0C the owner (a TerrainTypes::CellInstance, its first word is its matrix, or an ABaseEntity)
		Node*  m_apLinks[ 2 ]; // 0x10 the nodes the Z-jump goes on to
	};

	// A set of nodes (0x10 bytes)
	struct Set
	{
		// The point whose first node is a_pNode, or TNULL
		Point* FindPoint( const Node* a_pNode ) const;

		Node*  m_pNodes;      // 0x00
		Point* m_pPoints;     // 0x04 the points of the set
		TINT   m_iNodeCount;  // 0x08 (the assert's m_rSet.m_iNodeCount)
		TINT   m_iPointCount; // 0x0C
	};

	// A point the Z-jump starts from (0xC bytes)
	struct Point
	{
		Node* m_pNode;  // 0x00 the first node
		Set*  m_pSet;   // 0x04 (the assert's m_rSet)
		TINT  m_iUnk08; // 0x08 1: ABlobZJumpComponent's m_bUnk120
	};

	// The "ZJumpData" block (m_pData): the points at +0x00 and their count at +0x08, the sets at +0x04, their count
	// at +0x0C
	struct Block
	{
		Point* m_pPoints;     // 0x00
		Set*   m_pSets;       // 0x04
		TINT   m_iPointCount; // 0x08
		TINT   m_iSetCount;   // 0x0C
	};

public:
	AZJumpData( Toshi::TTRB* a_pTRB );
	// $deBlob: FUNCTION 005c8810
	~AZJumpData() = default;

	// The set a_iSet of the level's Z-jump data (0x10 bytes each, asserts "-1 < a_iSet", "a_iSet < m_iSetCount")
	Set* GetSet( TINT a_iSet ) const;

	// $deBlob: FUNCTION 0076a8f0
	Point* GetPoint( TINT a_iPoint ) const
	{
		Block* pBlock = TREINTERPRETCAST( Block*, m_pData );

		TASSERT( -1 < a_iPoint );
		TASSERT( a_iPoint < pBlock->m_iPointCount );

		return pBlock->m_pPoints + a_iPoint;
	}

	TINT GetPointCount() const { return TREINTERPRETCAST( Block*, m_pData )->m_iPointCount; }

	// The index of a_pPoint in the points
	TINT GetPointIndex( const Point* a_pPoint ) const { return TINT( a_pPoint - TREINTERPRETCAST( Block*, m_pData )->m_pPoints ); }

	// Starts (a_bOn) or stops the effect of the point a_iIndex (m_pUnkC)
	void SetPointEffect( TINT a_iIndex, TBOOL a_bOn );

	// Moves the effect of the point a_iIndex (if it has one) to the point's node (APropPlatform: the platforms move
	// their Z-jump nodes)
	void UpdatePointEffect( TINT a_iIndex );

	// Finds "Blob_Env_ZJump_Active" and adds an AEnvParticles emitter of it at each point (ALoadLevelState, once
	// the instances are created)
	void CreatePointEffects();

	// Removes the emitters of the points (AInGameState::OnRemoval, before the data is deleted)
	void DestroyPointEffects();

private:
	void*                   m_pData;      // 0x00 the "ZJumpData" symbol
	TINT                    m_iUnk4;      // 0x04
	Toshi::TParticleEffect* m_pEffect;    // 0x08 "Blob_Env_ZJump_Active" (the assert names it m_pEffect)
	AEnvEmitter**           m_apEmitters; // 0x0C the emitter of each point (TNULL: none)
};
