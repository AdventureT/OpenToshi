#pragma once
#include "TTMDBase.h"
#include "TSkeleton.h"
#include "Math/TSphere.h"

TOSHI_NAMESPACE_START

class TModelCollision;

//-----------------------------------------------------------------------------
// de Blob's PC model TRB data ("tmod"/"twld" TAssetInit entries).
// Barnyard's TTMDWin (inline vertices, TRBWinHeader with the LODs after it) is a different format.
//-----------------------------------------------------------------------------
namespace TTMDWin
{

struct TRBSkinSubMesh
{
	TUINT32  m_uiVertexCount;   // 0x00 44-byte vertices
	TUINT32  m_uiNumVertices2;  // 0x04 (16 bit in the mesh)
	TUINT32  m_uiIndexCount;    // 0x08
	TUINT32  m_uiBoneCount;     // 0x0C
	TUINT32* m_pBones;          // 0x10
	void*    m_pVertices;       // 0x14
	TUINT16* m_pIndices;        // 0x18
	TUINT32  m_aUnk[ 6 ];       // 0x1C copied to the mesh's sub mesh (+0x08..+0x1C)
}; // 0x34

struct TRBSkinMesh
{
	TUINT32         m_uiFlags;        // 0x00 passed to TSkinMesh::Create
	TUINT32         m_uiSubMeshCount; // 0x04
	TUINT32         m_uiIndexCount;   // 0x08
	TUINT32         m_uiVertexCount;  // 0x0C
	TCHAR*          m_szMaterialName; // 0x10
	TRBSkinSubMesh* m_pSubMeshes;     // 0x14
}; // 0x18

struct TRBLODInfo
{
	TINT                 m_iMeshCount1;  // 0x00
	TINT                 m_iMeshCount2;  // 0x04
	TTMDBase::SHADERTYPE m_eShader;      // 0x08 only SHADERTYPE_SKIN models load
	TSphere              m_RenderVolume; // 0x0C
}; // 0x1C

struct TRBLODHeader
{
	TRBLODInfo*   m_pInfo;       // 0x00
	TRBSkinMesh** m_ppMeshes;    // 0x04
	TUINT         m_uiMeshCount; // 0x08
};

struct TTRBWinHeader
{
	TCHAR*                    m_pTXSModelName;   // 0x00
	TUINT                     m_uiLODCount;      // 0x04
	TFLOAT                    m_fUnknown;        // 0x08 TModel::m_fUnknown
	TTMDBase::SkeletonHeader* m_pSkeletonHeader; // 0x0C
	TSkeleton*                m_pSkeleton;       // 0x10
	TModelCollision*          m_pModelCollision; // 0x14
	TRBLODHeader**            m_ppLODs;          // 0x18
};

// A mesh of a world cell: its bounding sphere and the TRB world mesh (TTRBWorldMesh, its m_pMesh created by
// TModelLoaderWorld_DX11)
struct TRBWorldCellMesh
{
	TSphere m_Sphere;     // 0x00
	void*   m_pWorldMesh; // 0x10 TTRBWorldMesh
};

// A node of a cell's sphere tree: a branch has its left child right after it (at +0x14) and its right child at
// m_pRight; a leaf (m_pRight TNULL) has the indices of its meshes in the cell
struct CellSphereTreeNode
{
	TSphere             m_Sphere;    // 0x00
	CellSphereTreeNode* m_pRight;    // 0x10
	TINT                m_iNumMeshes; // 0x14 (leaves)
	TINT16              m_aMeshIndices[ 1 ]; // 0x18 (leaves) m_iNumMeshes of them

	CellSphereTreeNode* GetLeft() { return TREINTERPRETCAST( CellSphereTreeNode*, TREINTERPRETCAST( TUINT8*, this ) + 0x14 ); }

	// The node following this leaf in memory
	CellSphereTreeNode* GetLeafEnd() { return TREINTERPRETCAST( CellSphereTreeNode*, ( TREINTERPRETCAST( TUINTPTR, this ) + m_iNumMeshes * 2 + 0x1B ) & ~TUINTPTR( 3 ) ); }
};

// A portal of a cell to other cells
struct TRBWorldPortal
{
	TUINT32   m_uiUnk00;     // 0x00
	TINT      m_iNumCells;   // 0x04
	TINT*     m_piCells;     // 0x08 the cells seen through the portal
	TVector3  m_Normal;      // 0x0C
	TINT      m_iNumVerts;   // 0x18
	TVector3* m_pVerts;      // 0x1C
};

// One cell of a world mesh tree
struct TRBWorldCell
{
	enum Flags : TUINT16
	{
		Flags_Building = 1, // being built by TWorldVis::Build
	};

	TUINT16          m_uiUnk00;          // 0x00
	TUINT16             m_uiFlags;          // 0x02 Flags
	TUINT8           m_aUnk04[ 0x6C ];   // 0x04
	TINT             m_iNumPortals;      // 0x70
	TRBWorldPortal** m_ppPortals;        // 0x74
	TUINT8           m_aUnk78[ 0x08 ];   // 0x78
	void*            m_pVisNode;         // 0x80 the TWorldVis::CellNode while built
	TINT             m_iMeshCount;       // 0x84
	TRBWorldCellMesh** m_ppMeshes;       // 0x88 the cell's meshes (indexed by the tree leaves)
	CellSphereTreeNode* m_pTreeRoot;     // 0x8C TModelLoaderWorld_DX11::LoadTreeIntersect
};

struct TRBWorldCellGroup
{
	TINT           m_iNumCells; // 0x00
	TRBWorldCell** m_ppCells;   // 0x04
};

struct TRBWorldData
{
	TINT                m_iNumGroups; // 0x00
	TRBWorldCellGroup** m_ppGroups;   // 0x04
};

struct TTRBWorldHeader
{
	TCHAR*                    m_pName;           // 0x00
	TRBWorldData*             m_pWorldData;      // 0x04
	TModelCollision*          m_pModelCollision; // 0x08
	TTMDBase::SkeletonHeader* m_pSkeletonHeader; // 0x0C
	TSkeleton*                m_pSkeleton;       // 0x10
	TFLOAT                    m_fUnknown;        // 0x14 TModel::m_fUnknown
	TSphere                   m_RenderVolume;    // 0x18
};

}; // namespace TTMDWin

TOSHI_NAMESPACE_END
