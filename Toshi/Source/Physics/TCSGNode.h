#pragma once
#include "TShapeBaseClass.h"
#include "Math/TMatrix44.h"

TOSHI_NAMESPACE_START

// A shape of a shape list (TPhysicsObjectContainer shape type 7) placed by a transform; reference counted and
// shared through a cache like the shape data
class TCSGNode
{
public:
	TCSGNode()
	    : m_pShapeData( TNULL ), m_iRefCount( 0 )
	{}

	// $deBlob: FUNCTION 00733be0
	// $deBlob: FUNCTION 00733850
	virtual ~TCSGNode() = default;

	// Returns the cached node of a_pShapeData (of type a_eType) at a_rcTransform (creating it if needed) with one
	// more reference; spheres compare only the translation, capsules and cylinders also the forward axis
	static TCSGNode* Get( TINT a_eType, TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcTransform );

	// Removes a reference; the last one removes the node from the cache and deletes it
	void Release();

	const TMatrix44& GetTransform() const { return m_oTransform; }
	TShapeBaseClass* GetShapeData() const { return m_pShapeData; }

public:
	static T2Vector<TCSGNode*, 200> ms_oCache;

private:
	TMatrix44        m_oTransform; // 0x04
	TShapeBaseClass* m_pShapeData; // 0x44
	TINT             m_iRefCount;  // 0x48
};

TOSHI_NAMESPACE_END
