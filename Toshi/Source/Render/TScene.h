#pragma once
#include "Toshi/T2DList.h"
#include "Math/TMatrix44.h"

TOSHI_NAMESPACE_START

class TModelInstance;

//-----------------------------------------------------------------------------
// Purpose: the scene of ARenderer: owns a pool of 800 nodes that record a model instance and its model view matrix,
// queued to be rendered before or after the depth buffer is cleared.
//-----------------------------------------------------------------------------
class TScene
{
public:
	static TCONSTEXPR_VAR TINT MAX_MODEL_INSTANCES = 800;

	struct ModelInstanceNode : public T2DList<ModelInstanceNode>::Node
	{
		// $deBlob: FUNCTION 006b4430
		~ModelInstanceNode() = default;

		TModelInstance* m_pModelInstance; // 0x08
		TMatrix44       m_oModelView;     // 0x0C
	};

public:
	TScene();
	~TScene();

	// Puts the queued nodes back to the free list
	void Reset();

	// The scene camera: a model instance in the view frustum is queued with its model view matrix
	// (before the depth buffer clear if the instance is flagged 0x10), RenderSceneCamera updates their skeletons and
	// renders them. RenderSceneCamera's arguments aren't read.
	static void AddToSceneCamera( TModelInstance* a_pModelInstance );
	static void RenderSceneCamera( const TMatrix44& a_rUnk1, const TMatrix44& a_rUnk2 );
	static void RenderMeshes( T2DList<ModelInstanceNode>* a_pList );
	static void UpdateSkeletonStates( T2DList<ModelInstanceNode>* a_pList );
	static void UpdateSkeletonStatesTest( T2DList<ModelInstanceNode>* a_pList );
	static void PrintRenderingMeshes( T2DList<ModelInstanceNode>* a_pList );

private:
	TUINT8 m_aUnk00[ 0x14 ]; // 0x00 never used by de Blob

	static ModelInstanceNode*         g_pModelInstances;
	static T2DList<ModelInstanceNode> g_oFreeModelInstances;
	static T2DList<ModelInstanceNode> g_oBeforeZBufferClearModelInstances;
	static T2DList<ModelInstanceNode> g_oAfterZBufferClearModelInstances;
};

TOSHI_NAMESPACE_END
