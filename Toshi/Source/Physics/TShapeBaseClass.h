#pragma once
#include "Toshi/T2Vector.h"

TOSHI_NAMESPACE_START

class TShapeBaseClass;

// The caches of shared shape data (one per shape type, e.g. TBoxData's)
using TShapeDataCache = T2Vector<TShapeBaseClass*, 200>;

// Reference counted shape data shared by physics object containers
class TShapeBaseClass
{
public:
	enum Type : TINT
	{
		Type_None        = 0,
		Type_Plane       = 1,
		Type_Sphere      = 2,
		Type_Box         = 3,
		Type_Capsule     = 4,
		Type_Cylinder    = 5,
		Type_Mesh        = 6, // TPhysicsMeshContainer
		Type_List        = 7, // TShapeList
		Type_HeightField = 8, // TPhysicsHFContainer
	};

public:
	// $deBlob: FUNCTION 007331f0
	TShapeBaseClass()
	    : m_eType( Type_None ), m_iRefCount( 0 ), m_pCache( TNULL )
	{}

	virtual ~TShapeBaseClass();

	// Adds a reference; the first one registers the data in a_pCache
	virtual void AddRef( TShapeDataCache* a_pCache ); // +0x4

	// Removes a reference; the last one removes the data from its cache and deletes it
	virtual void Release(); // +0x8

	TINT GetType() const { return m_eType; }

protected:
	TINT             m_eType;     // 0x4
	TINT             m_iRefCount; // 0x8
	TShapeDataCache* m_pCache;    // 0xc
};

TOSHI_NAMESPACE_END
