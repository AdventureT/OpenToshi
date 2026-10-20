#pragma once
#include "Math/TVector3.h"
#include "Math/TVector4.h"
#include "Math/TPlane.h"
#include "Math/TSphere.h"
#include "Math/TMatrix44.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a bounding box hierarchy of items, rendered by culling the boxes against the frustum planes (or a sphere).
// The tree is either built from added items (AddItem, Build) or set to a prebuilt tree (SetRoot, e.g. the instance trees
// of the terrain cells).
//-----------------------------------------------------------------------------
class TBoxTree
{
public:
	// A box: the centre and the half extents (TAABox)
	struct Box
	{
		// The result of ClassifyPlane
		enum PlaneSide : TINT
		{
			PlaneSide_Behind   = -1,
			PlaneSide_Crossing = 0,
			PlaneSide_Front    = 1,
		};

		// The result of ClassifyClip
		enum ClipResult : TINT
		{
			ClipResult_Outside  = 0,
			ClipResult_Inside   = 1,
			ClipResult_Crossing = 2,
		};

		// The bits of the clip code of a corner of the box
		enum ClipCode : TUINT
		{
			ClipCode_XMin = 0x01,
			ClipCode_XMax = 0x02,
			ClipCode_YMin = 0x04,
			ClipCode_YMax = 0x08,
			ClipCode_ZMax = 0x10,
			ClipCode_ZMin = 0x20,
		};

		TINT  ClassifyPlane( const TPlane& a_rcPlane ) const;
		TBOOL IsPointInside( const TVector3& a_rcPoint ) const;
		void  SetUnion( const Box* a_pBoxes, TUINT a_uiNumBoxes );
		TINT  ClassifyClip( const TMatrix44& a_rcMatrix ) const;
		// $deBlob: FUNCTION 0068c250
		const TVector3& GetExtents() const { return vExtents; }

		TVector3 vCentre;  // 0x00
		TVector3 vExtents; // 0x0C
	}; // 0x18

	struct Node
	{
		Node* pLeft;  // 0x00 TNULL: a leaf
		Node* pRight; // 0x04
		void* pItem;  // 0x08 (leaves)
		Box   oBox;   // 0x0C
	}; // 0x24

	// An entry of the list of nodes not in the tree yet
	struct ListItem
	{
		Node*     pNode; // 0x00
		ListItem* pNext; // 0x04
		ListItem* pPrev; // 0x08
	}; // 0x0C

	enum Flags : TUINT8
	{
		Flags_SortByDistance = 1 << 0, // the nearer child is rendered first (to m_vViewPos)
		Flags_Sphere         = 1 << 1, // culled against m_pSphere instead of m_pPlanes
	};

	using t_RenderFunction   = void ( * )( void* a_pItem, void* a_pUserData );
	using t_MakeAABBFunction = Box* ( * )( Box* a_pOutBox, void* a_pItem );

public:
	TBoxTree();
	~TBoxTree();

	// Adds an item (its box from m_pMakeAABBFunction) to the list of nodes to build
	void AddItem( void* a_pItem );

	// Builds the tree from the added items (merges the pairs with the smallest union box first)
	void Build();

	// Renders the items of the tree that intersect the planes/sphere
	void Render();

	void SetRoot( Node* a_pRoot ) { m_pRoot = a_pRoot; }
	// $deBlob: FUNCTION 005d1680
	void SetRenderFunction( t_RenderFunction a_fnRender ) { m_pRenderFunction = a_fnRender; }
	// $deBlob: FUNCTION 005d1600
	void SetUserData( void* a_pUserData ) { m_pUserData = a_pUserData; }
	// $deBlob: FUNCTION 005d1690
	void SetPlanes( const TPlane* a_pPlanes ) { m_pPlanes = a_pPlanes; }
	void SetViewPosition( const TVector4& a_rPosition ) { m_vViewPos = a_rPosition; }

	void SetSphere( const TSphere* a_pSphere ) { m_pSphere = a_pSphere; }

	const TPlane*    GetPlanes() const { return m_pPlanes; }
	const TSphere*   GetSphere() const { return m_pSphere; }
	void*            GetUserData() const { return m_pUserData; }
	t_RenderFunction GetRenderFunction() const { return m_pRenderFunction; }
	TUINT8           GetFlags() const { return m_uiFlags; }

	void SetFlag( TUINT8 a_uiFlag ) { m_uiFlags |= a_uiFlag; }
	void ClearFlag( TUINT8 a_uiFlag ) { m_uiFlags &= ~a_uiFlag; }

private:
	void AddListItem( ListItem* a_pItem );
	void RemoveListItem( ListItem* a_pItem );

	void RenderPlanes( Node* a_pNode, TUINT a_uiClipFlags );
	void RenderSphere( Node* a_pNode );

	static void DeleteNode( Node* a_pNode );

private:
	TVector4           m_vViewPos;           // 0x00
	const TPlane*      m_pPlanes;            // 0x10
	const TSphere*     m_pSphere;            // 0x14
	void*              m_pUserData;          // 0x18 passed to m_pRenderFunction
	ListItem*          m_pItems;             // 0x1C
	Node*              m_pRoot;              // 0x20
	TBOOL              m_bBuilt;             // 0x24 the tree was built (and is owned)
	t_RenderFunction   m_pRenderFunction;    // 0x28
	t_MakeAABBFunction m_pMakeAABBFunction;  // 0x2C
	TUINT32            m_uiUnk30;            // 0x30 not initialised
	TUINT8             m_uiFlags;            // 0x34 Flags
};

TOSHI_NAMESPACE_END
