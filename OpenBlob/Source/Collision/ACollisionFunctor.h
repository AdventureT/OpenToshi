#pragma once
#include <Math/TVector4.h>

class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: the callbacks of a collision query (ACollisionManager2::SweepAABB...): filters the polygons and objects
// that are hit and is told about the accepted ones.
// de Blob's PreFilterSweptAABBPoly has no TUINT parameter and AABBSweptHitPoly sits at +0x1C
//-----------------------------------------------------------------------------
class ACollisionFunctor
{
public:
	// Returns TTRUE to reject the polygon (a_pTriangle: its 3 vertices, TNULL when not a mesh)
	// $deBlob: FUNCTION 005acbc0
	virtual TBOOL PreFilterSweptAABBPoly( const Toshi::TVector4& a_rNormal, const Toshi::TVector4* a_pTriangle ) { return TFALSE; } // +0x00
	// $deBlob: FUNCTION 005acbd0
	virtual TBOOL PreFilterSweptSpherePoly( const Toshi::TVector4& a_rNormal ) { return TFALSE; } // +0x04
	// $deBlob: FUNCTION 005acbb0
	virtual TBOOL PreFilterSpherePoly( const Toshi::TVector4& a_rNormal ) { return TFALSE; } // +0x08
	// $deBlob: FUNCTION 005acba0
	virtual TBOOL PreFilterAABBPoly( const Toshi::TVector4& a_rNormal ) { return TFALSE; } // +0x0C
	// $deBlob: FUNCTION 005acb80
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) { return TFALSE; } // +0x10
	// $deBlob: FUNCTION 005acb60
	virtual void HitGroup( const TCHAR* a_szGroupName, TUINT32 a_eGroupFlag ) {} // +0x14
	// Returns TTRUE to skip the object
	// $deBlob: FUNCTION 005acb70
	virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) { return TFALSE; } // +0x18
	// $deBlob: FUNCTION 005acb40
	virtual void AABBSweptHitPoly( const Toshi::TVector4* a_pTriangle, TFLOAT a_fDistance ) {} // +0x1C
	virtual ~ACollisionFunctor() = default;                                                    // +0x20
};
