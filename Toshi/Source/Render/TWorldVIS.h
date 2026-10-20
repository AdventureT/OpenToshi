#pragma once
#include "Render/TTMDWin.h"
#include "Math/TMatrix44.h"
#include "Math/TPlane.h"
#include "Math/TSphere.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: the clipping planes a TWorldVis tests the cells' sphere trees against, with the planes still active for the
// current node.
//-----------------------------------------------------------------------------
struct Frustum
{
	struct ClipPlane
	{
		TPlane  m_Plane;      // 0x00
		TUINT32 m_uiClipFlag; // 0x10
	};

	// The result of the sphere tests
	enum SphereTest : TINT
	{
		SphereTest_Inside       = 0,
		SphereTest_Outside      = 1,
		SphereTest_Intersecting = 2,
	};

	// Transforms the planes of a_rSource by a_rMatrix (orthogonal)
	void Transform( const Frustum& a_rSource, const TMatrix44& a_rMatrix );

	// Makes every plane active
	void InitReduce();

	// Returns 0 if the sphere is inside the active planes, 1 if it's outside of one of them and 2 if it intersects;
	// the planes the sphere is fully inside of are deactivated
	TINT IntersectSphereReduce( const TSphere& a_rSphere );

	// The same test against all 6 planes without deactivating any (0 inside, 1 outside, 2 intersecting)
	TINT IntersectSphere( const TSphere& a_rSphere );

	// The clip flags of the active planes
	TUINT32 GetActiveClipFlags();

	TUINT8     m_aUnk00[ 0x0C ];         // 0x00
	ClipPlane  m_aPlanes[ 6 ];           // 0x0C
	ClipPlane* m_apPlanes[ 6 ];          // 0x84 the active ones first
	TINT       m_iActivePlaneCount;      // 0x9C
};

//-----------------------------------------------------------------------------
// Purpose: the visibility of a world model (a group of TRB world cells linked by portals): Build finds the cells seen
// from the camera through the portals, Render walks the sphere trees of those cells and calls the render callback for
// every cell mesh in the frustum.
//-----------------------------------------------------------------------------
class TWorldVis
{
public:
	static TCONSTEXPR_VAR TINT MAX_VISIBLE_CELLS = 1;

	enum Flags : TUINT32
	{
		Flags_Transform = 1, // m_oTransform is used
	};

	// The screen rectangle a cell is seen through
	struct CullBox
	{
		TFLOAT m_fMinX; // 0x00
		TFLOAT m_fMinY; // 0x04
		TFLOAT m_fMaxX; // 0x08
		TFLOAT m_fMaxY; // 0x0C
	};

	struct CellSettings
	{
		CellSettings* m_pNext;    // 0x00
		CullBox       m_CullBox;  // 0x04
		TUINT32       m_uiUnk14;  // 0x14
	};

	// A built cell (pointed to by its TRBWorldCell::m_pVisNode while built)
	struct CellNode
	{
		TINT          m_iCellIndex; // 0x00
		CellSettings* m_pSettings;  // 0x04
		TUINT32       m_uiUnk08;    // 0x08
	};

	struct RenderData
	{
		TTMDWin::TRBWorldCell* m_pCell;    // 0x00
		Frustum*               m_pFrustum; // 0x04
	};

	using t_RenderCallback     = void( __stdcall* )( TTMDWin::TRBWorldCellMesh* a_pCellMesh, RenderData* a_pRenderData );
	using t_RenderCallbackUser = void( __stdcall* )( TTMDWin::TRBWorldCellMesh* a_pCellMesh, RenderData* a_pRenderData, void* a_pUserData );

	struct StackValue
	{
		TTMDWin::CellSphereTreeNode* m_pNode;              // 0x00
		TTMDWin::CellSphereTreeNode* m_pParent;            // 0x04 its m_pRight ends the node's subtree
		TINT                         m_iActivePlaneCount;  // 0x08
	};

	static TCONSTEXPR_VAR TINT s_iStackSize = 100;

public:
	TWorldVis();
	~TWorldVis();

	// a_pTransform (optional) transforms the spheres of the cells
	void Create( TTMDWin::TRBWorldCellGroup* a_pWorld, const TMatrix44* a_pTransform );
	void Destroy();
	void Reset();

	// Builds the cells seen from a_iCellIndex with a_rModelView (inverted as orthonormal unless a_bInvert) on the
	// whole display
	void Build( const TMatrix44& a_rModelView, const TMatrix44& a_rProjection, TINT a_iCellIndex, TBOOL a_bInvert );

	// Builds a_iCellIndex seen through a_pCullBox and the cells seen through its portals
	void Build( const TMatrix44& a_rModelView, const TMatrix44& a_rProjection, TINT a_iCellIndex, const CullBox* a_pCullBox );

	void Render( const TMatrix44& a_rModelView, const TMatrix44& a_rProjection, TBOOL a_bInvert );
	// Render's frustum set up for the settings of every built cell, without rendering (the frustums are dropped)
	void TransformFrustums( const TMatrix44& a_rModelView, TINT a_iUnused );

	void RenderTreeIntersectNonRecurse( TTMDWin::CellSphereTreeNode* a_pNode, RenderData* a_pRenderData );
	void RenderLeafNodeIntersect( TTMDWin::CellSphereTreeNode* a_pNode, RenderData* a_pRenderData );
	void RenderLeafNodeIn( TTMDWin::CellSphereTreeNode* a_pNode, RenderData* a_pRenderData );

	// $deBlob: FUNCTION 00558360
	void SetRenderCallback( t_RenderCallback a_fnCallback )
	{
		TASSERT( m_pfnRenderCallbackUser == TNULL );
		m_pfnRenderCallback = a_fnCallback;
	}

	// $deBlob: FUNCTION 005550a0
	void SetRenderCallbackUser( t_RenderCallbackUser a_fnCallback, void* a_pUserData )
	{
		TASSERT( m_pfnRenderCallback == TNULL );
		m_pfnRenderCallbackUser     = a_fnCallback;
		m_pRenderCallbackUserData   = a_pUserData;
	}

private:
	static StackValue* s_pStack;
	static TVector4    avScreen[ 8 ]; // the portal's vertices on the screen

public:
	TTMDWin::TRBWorldCellGroup* m_pWorld;                             // 0x00
	CellSettings                m_aCellSettings[ MAX_VISIBLE_CELLS ]; // 0x04
	CellNode*                   m_pCellNodes;                         // 0x1C
	TINT                        m_iNumBuiltCells;                     // 0x20
	TINT                        m_iNumCellSettings;                   // 0x24
	TUINT32                     m_uiUnk28;                            // 0x28
	TVector3                    m_ViewModelPos;                       // 0x2C
	t_RenderCallback            m_pfnRenderCallback;                  // 0x38
	t_RenderCallbackUser        m_pfnRenderCallbackUser;              // 0x3C
	void*                       m_pRenderCallbackUserData;            // 0x40
	TUINT32                     m_uiFlags;                            // 0x44 Flags
	TMatrix44                   m_oTransform;                         // 0x48
};

TOSHI_NAMESPACE_END
