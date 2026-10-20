#include "ToshiPCH.h"
#include "Render/TScene.h"
#include "Render/TModel.h"
#include "Render/TRender.h"
#include "Render/TRenderContext.h"
#include "Render/TMesh.h"
#include "Render/TSkeleton.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TScene::ModelInstanceNode* TScene::g_pModelInstances;
// $deBlob: FUNCTION 0053ed10
// $deBlob: FUNCTION 00858fe0
T2DList<TScene::ModelInstanceNode> TScene::g_oFreeModelInstances;
// $deBlob: FUNCTION 0053ecf0
// $deBlob: FUNCTION 00858fd0
T2DList<TScene::ModelInstanceNode> TScene::g_oBeforeZBufferClearModelInstances;
// $deBlob: FUNCTION 0053ecd0
// $deBlob: FUNCTION 00858fc0
T2DList<TScene::ModelInstanceNode> TScene::g_oAfterZBufferClearModelInstances;

// $deBlob: FUNCTION 006b3c70
TScene::TScene()
{
	TASSERT( g_pModelInstances == TNULL );
	g_pModelInstances = new ModelInstanceNode[ MAX_MODEL_INSTANCES ];

	for ( TINT i = 0; i < MAX_MODEL_INSTANCES; i++ )
		g_oFreeModelInstances.PushBack( &g_pModelInstances[ i ] );
}

// $deBlob: FUNCTION 006b3d20
// $deBlob: FUNCTION 005f01d0
TScene::~TScene()
{
	Reset();
	g_oFreeModelInstances.Clear();

	if ( g_pModelInstances )
		delete[] g_pModelInstances;

	g_pModelInstances = TNULL;
}

// $deBlob: FUNCTION 006b3e50
void TScene::Reset()
{
	while ( !g_oBeforeZBufferClearModelInstances.IsEmpty() )
	{
		ModelInstanceNode* pNode = g_oBeforeZBufferClearModelInstances.Head();
		pNode->Remove();
		g_oFreeModelInstances.PushFront( pNode );
	}

	while ( !g_oAfterZBufferClearModelInstances.IsEmpty() )
	{
		ModelInstanceNode* pNode = g_oAfterZBufferClearModelInstances.Head();
		pNode->Remove();
		g_oFreeModelInstances.PushFront( pNode );
	}
}

// $deBlob: FUNCTION 006b3d60
void TScene::AddToSceneCamera( TModelInstance* a_pModelInstance )
{
	if ( g_oFreeModelInstances.IsEmpty() || !a_pModelInstance->IsInViewFrustum() )
		return;

	ModelInstanceNode* pNode = g_oFreeModelInstances.PopFront();
	pNode->m_pModelInstance  = a_pModelInstance;

	a_pModelInstance->GetTransform().Push();
	pNode->m_oModelView = TRender::GetSingleton()->GetCurrentContext()->GetModelViewMatrix();
	a_pModelInstance->GetTransform().Pop();

	if ( a_pModelInstance->GetFlags() & TModelInstance::Flags_BeforeZBufferClear )
		g_oBeforeZBufferClearModelInstances.PushBack( pNode );
	else
		g_oAfterZBufferClearModelInstances.PushBack( pNode );
}

// $deBlob: FUNCTION 006b3f30
void TScene::RenderSceneCamera( const TMatrix44& a_rUnk1, const TMatrix44& a_rUnk2 )
{
	if ( g_oBeforeZBufferClearModelInstances.IsEmpty() && g_oAfterZBufferClearModelInstances.IsEmpty() )
		return;

	UpdateSkeletonStates( &g_oBeforeZBufferClearModelInstances );
	UpdateSkeletonStates( &g_oAfterZBufferClearModelInstances );

	TRenderContext* pRenderContext = TRender::GetSingleton()->GetCurrentContext();
	TMatrix44       mModelView     = pRenderContext->GetModelViewMatrix();

	RenderMeshes( &g_oBeforeZBufferClearModelInstances );
	RenderMeshes( &g_oAfterZBufferClearModelInstances );

	pRenderContext->SetModelViewMatrix( mModelView );
}

// $deBlob: FUNCTION 006b4170
void TScene::RenderMeshes( T2DList<ModelInstanceNode>* a_pList )
{
	TRenderContext* pRenderContext = TRender::GetSingleton()->GetCurrentContext();

	for ( auto it = a_pList->Begin(); it != a_pList->End(); it++ )
	{
		TModelInstance* pModelInstance = it->m_pModelInstance;
		TINT            iLOD           = pModelInstance->GetLOD();

		TModel*    pModel = TSTATICCAST( TModel, T2ModelPtr( pModelInstance->GetModel() ).GetData() );
		TModelLOD& rLOD   = pModel->GetLOD( iLOD );

		pRenderContext->SetSkeletonInstance( pModelInstance->GetSkeletonInstance() );
		pRenderContext->SetModelViewMatrix( it->m_oModelView );

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
			rLOD.GetMeshes()[ i ]->Render();
	}
}

// $deBlob: FUNCTION 006b40f0
void TScene::UpdateSkeletonStates( T2DList<ModelInstanceNode>* a_pList )
{
	for ( auto it = a_pList->Begin(); it != a_pList->End(); it++ )
	{
		TSkeletonInstance* pSkeletonInstance = it->m_pModelInstance->GetSkeletonInstance();

		if ( pSkeletonInstance != TNULL && TINT16( pSkeletonInstance->GetSkeleton()->GetAutoBoneCount() ) < 0x80 )
			pSkeletonInstance->UpdateState( TFALSE );
	}
}

// $deBlob: FUNCTION 006b4090
void TScene::UpdateSkeletonStatesTest( T2DList<ModelInstanceNode>* a_pList )
{
	for ( auto it = a_pList->Begin(); it != a_pList->End(); it++ )
	{
		if ( it->m_pModelInstance->GetSkeletonInstance() != TNULL )
			it->m_pModelInstance->GetSkeletonInstance()->UpdateState( TFALSE );
	}
}

// $deBlob: FUNCTION 006b3fe0
void TScene::PrintRenderingMeshes( T2DList<ModelInstanceNode>* a_pList )
{
	// NOTE: retail's printing is compiled out, only the iteration's asserts are left
	for ( auto it = a_pList->Begin(); it != a_pList->End(); it++ )
		TASSERT( it.Get() != TNULL );
}

TOSHI_NAMESPACE_END
