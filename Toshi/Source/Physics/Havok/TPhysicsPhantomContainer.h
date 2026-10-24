#pragma once
#include "Toshi/T2DList.h"

class hkpPhantom;

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a Havok phantom owned by TPhysicsManager (0x14 bytes, in its list at 0x108): an AABB phantom (ray casts) or
// a shape phantom (shape queries) that is moved to each query. Created by TPhysicsManager::CreateShapePhantom/
// CreateAabbPhantom.
//-----------------------------------------------------------------------------
class TPhysicsPhantomContainer
    : public T2DList<TPhysicsPhantomContainer>::Node
{
public:
	enum TPhantomType
	{
		TPhantomType_AABB,
		TPhantomType_Simple,
		TPhantomType_Caching,
	};

	enum Flags : TUINT32
	{
		PPF_AABB          = 1,
		PPF_SIMPLE        = 2,
		PPF_CACHING       = 4,
		PPF_IN_SIMULATION = 8,
	};

public:
	TPhysicsPhantomContainer()
	    : m_uiWorld( 0 ), m_shapeType( 0 ), m_uiFlags( 0 ), m_phantom( TNULL )
	{}

	static TPhysicsPhantomContainer* Create();
	static void                      Destroy( TPhysicsPhantomContainer* a_pContainer );

	// $deBlob: FUNCTION 00561040
	TPhantomType GetType() const
	{
		if ( m_uiFlags & PPF_AABB ) return TPhantomType_AABB;
		if ( m_uiFlags & PPF_SIMPLE ) return TPhantomType_Simple;
		return TPhantomType( ( ( m_uiFlags & 0xFF ) >> 1 ) & 2 );
	}

	// Inlined in TPhysicsManager::CreateShapePhantom/CreateAabbPhantom/DestroyPhantom
	// $deBlob: FUNCTION 007313e0
	void SetType( TINT a_eType )
	{
		if ( a_eType == TPhantomType_AABB )
			m_uiFlags = ( m_uiFlags & ~6 ) | PPF_AABB;
		else if ( a_eType == TPhantomType_Simple )
			m_uiFlags = ( m_uiFlags & ~5 ) | PPF_SIMPLE;
		else if ( a_eType == TPhantomType_Caching )
			m_uiFlags = ( m_uiFlags & ~3 ) | PPF_CACHING;
		else
			m_uiFlags &= ~7;
	}

	TBOOL IsInSimulation() const { return ( m_uiFlags & PPF_IN_SIMULATION ) != 0; }

public:
	TUINT16     m_uiWorld;   // 0x08 the TPhysicsManager world
	TUINT16     m_shapeType; // 0x0a TShapeBaseClass type of a shape phantom
	TUINT32     m_uiFlags;   // 0x0c PPF_*
	hkpPhantom* m_phantom;   // 0x10
};

TOSHI_NAMESPACE_END
