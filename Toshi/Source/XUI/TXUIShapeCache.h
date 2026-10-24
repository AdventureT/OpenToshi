#pragma once
#include "TXUIElement.h"
#include "Math/TVector2.h"
#include "Toshi/TSingleton.h"
#include "Toshi/T2Allocator.h"
#include "Toshi/T2DList.h"
#include "Toshi/T2Map.h"

TOSHI_NAMESPACE_START

struct XUIFigurePoint;
class T2GUITransform;

// Evaluates the Bezier curve of the a_iN + 1 control points at a_fT
TVector2 Bezier( const TVector2* a_pPoints, TINT a_iN, TFLOAT a_fT );

//-----------------------------------------------------------------------------
// The key of a cached shape: the points of a figure (owned by its XURXUIFigureData) (0x8)
//-----------------------------------------------------------------------------
struct TXUIShapeParams
{
	const XUIFigurePoint* m_pPoints;     // 0x0
	TUINT32               m_uiNumPoints; // 0x4
};

//-----------------------------------------------------------------------------
// A tesselated figure path (0x24): the vertices along the path (Bezier segments subdivided), the triangle list
// of its fill (ear clipping) and, for the anti-aliasing edges made by CloneAndMakeAaEdge, a tri strip with colours
// and UVs
//-----------------------------------------------------------------------------
class TXUIShape
{
public:
	// A vertex of the polygon being ear clipped (0xC)
	class VertNode : public T2DList<VertNode>::Node
	{
	public:
		VertNode() {}

		// $deBlob: FUNCTION 006dd110
		~VertNode() {}

	public:
		TUINT16 m_uiIndex; // 0x8
	};

public:
	TXUIShape();
	~TXUIShape();

	TBOOL      CreateControlPoints( const XUIFigurePoint* a_pPoints, TUINT32 a_uiNumPoints );
	void       CalcScale();
	void       TesselateShape( const XUIFigurePoint* a_pPoints, TUINT32 a_uiNumPoints );
	TBOOL      CanTesselate();
	TBOOL      IsEar( TUINT32 a_uiPrevVert, TUINT32 a_uiCurrVert, TUINT32 a_uiNextVert );
	TBOOL      TesselateStroke( TFLOAT a_fWidth, TVector2*& a_rpVerts, TVector2 a_vScale );
	TXUIShape* CloneAndMakeAaEdge( TVector2 a_vScale, TUINT32 a_uiColour );
	void       MakeAaEdgeVerts( TXUIShape* a_pShape, TVector2 a_vScale );
	void       SetColorAaEdge( TUINT32 a_uiColour );
	void       MakeUVsAaEdge( T2GUITransform* a_pTransform, TVector2 a_vScale, TVector2 a_vSize, TFLOAT a_fMaxU, TFLOAT a_fMaxV );
	void       ComputeIsClockWise();

	static TBOOL IsDegenerateSpline( TVector2* a_pControlPts );
	static TINT  CalculateSplineSteps( TVector2* a_pControlPts );

public:
	// Number of vertices of a long Bezier segment (scaled down for the short ones, at least 2)
	// . The game raises it to 20 while it creates lifelost.xur's scene
	static TINT ms_iSplineSteps;

public:
	TXUIFixedVector2<6>    m_vDims;               // 0x00 size of the bounding box (the vertices are centred on it)
	TXUIFixedVector2<6>*   m_pVerts;              // 0x04
	TXUIFixedVector2<6>*   m_pControlPoints;      // 0x08 3 per figure point
	TUINT16*               m_pIndices;            // 0x0C
	TUINT32*               m_pColours;            // 0x10 anti-aliasing edges
	TXUIFixedVector2<128>* m_pUVs;                // 0x14 anti-aliasing edges
	TUINT16                m_uiRefCount;          // 0x18
	TUINT16                m_uiNumControlPoints;  // 0x1A
	TUINT16                m_uiNumVerts;          // 0x1C
	TUINT16                m_uiNumIndices;        // 0x1E
	TUINT16                m_uiNumTriangles;      // 0x20
	TBOOL                  m_bIsClockWise;        // 0x22
};

//-----------------------------------------------------------------------------
// Cache of the tesselated shapes of the XuiFigures (0x24, Source\Xui\TXUIShapeCache.cpp): the figures with the
// same points share a shape (refcounted)
//-----------------------------------------------------------------------------
class TXUIShapeCache : public TSingleton<TXUIShapeCache>
{
public:
	struct Comparator
	{
		TINT operator()( TXUIShapeParams* a_pParams1, TXUIShapeParams* a_pParams2 ) const;
	};

	// NOTE: retail's member is a T2MemoryAllocator (a T2Allocator on a TMemoryHeap), which the engine doesn't have yet.
	// This copy does the same (heap Malloc/Memalign, TFree, CanAllocate TTRUE, sizes 0); retail's vtable has a 9th slot, Realloc (TRealloc), that the engine's T2Allocator lacks
	class HeapAllocator : public T2Allocator
	{
	public:
		HeapAllocator( TMemoryHeap* a_pHeap )
		    : m_pHeap( a_pHeap ) {}

		virtual void*  Malloc( size_t a_uiSize ) override;
		virtual void*  Malloc( size_t a_uiSize, size_t a_uiAlignment ) override;
		virtual void   Free( void* a_pMem ) override;
		virtual TBOOL  CanAllocate( size_t a_uiSize ) const override;
		virtual void*  TryMalloc( size_t a_uiSize ) override;
		virtual void*  TryMalloc( size_t a_uiSize, size_t a_uiAlignment ) override;
		virtual size_t GetUsedSize() const override;
		virtual size_t GetCapacity() const override;

	private:
		TMemoryHeap* m_pHeap; // 0x4
	};

	using ShapeMap = T2Map<TXUIShapeParams*, TXUIShape*, Comparator>;

public:
	TXUIShapeCache();
	~TXUIShapeCache();

	// Returns the shape of a_pParams' points (creates and tesselates it if needed). Takes the ownership of a_pParams.
	TXUIShape* CreateShape( TXUIShapeParams* a_pParams );

	// Releases a shape returned by CreateShape
	void DestroyShape( TXUIShape* a_pShape );

private:
	HeapAllocator m_oAllocator;         // 0x00
	ShapeMap      m_oFigureToShapeMap;  // 0x08
};

TOSHI_NAMESPACE_END
