#include "pch.h"
#include "ASpawnedCivManager.h"

#include "APooledEntityManager.h"
#include "ABaseEntity.h"
#include "ACiv.h"
#include "../A2DHash.h"
#include "../Cameras/ACamera.h"
#include "../Render/ARenderer.h"
#include "../Render/AXYZViewportManager.h"
#include "../Terrain2/ATerrain2.h"

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ASpawnedCivManager ) == 0x30 );
TSTATICASSERT( sizeof( APropPlatform::RespawnData ) == 0x2C );

// The offset of the list node in an A2DHashItem (and so in an ACivWrapper)
static TCONSTEXPR_VAR TINT ITEM_NODE_OFFSET = 0x0C;

//-----------------------------------------------------------------------------
// Purpose: the item of a list node (the node is at 0x0C of the item)
//-----------------------------------------------------------------------------
static A2DHashItem* GetItemOfNode( Toshi::T2GenericDList::Node* a_pNode )
{
	return TREINTERPRETCAST( A2DHashItem*, TREINTERPRETCAST( TUINT8*, a_pNode ) - ITEM_NODE_OFFSET );
}

// $deBlob: FUNCTION 007627c0
ASpawnedCivManager::ASpawnedCivManager()
    : m_paCivWrappers( TNULL )
    , m_iNumWrappers( 0 )
    , m_iNumCivs( 0 )
    , m_pHash( TNULL )
{
}

// $deBlob: FUNCTION 00762890
// $deBlob: FUNCTION 0078daf0
ASpawnedCivManager::~ASpawnedCivManager()
{
	ReleaseAll();

	// The free wrappers are dropped with the array
	while ( m_aCivLists[ 3 ].Next() != &m_aCivLists[ 3 ] )
		m_aCivLists[ 3 ].Next()->Remove();

	if ( m_paCivWrappers != TNULL )
	{
		delete[] m_paCivWrappers;
		m_paCivWrappers = TNULL;
	}

	if ( m_pHash != TNULL )
	{
		delete m_pHash;
		m_pHash = TNULL;
	}
}

// $deBlob: FUNCTION 00762990
TBOOL ASpawnedCivManager::Create()
{
	TASSERT( m_paCivWrappers == TNULL );

	m_iNumWrappers = m_iNumCivs;

	if ( m_iNumWrappers > 0 )
	{
		m_paCivWrappers = new ACivWrapper[ m_iNumWrappers ];

		// Every wrapper starts free
		for ( TINT i = 0; i < m_iNumWrappers; i++ )
			m_paCivWrappers[ i ].m_Node.InsertBefore( &m_aCivLists[ 3 ] );
	}

	TFLOAT fMinX, fMinY, fMaxX, fMaxY;
	TINT   iXDiv, iYDiv;

	A2DHash::GetLevelBounds( 16.0f, &fMinX, &fMinY, &fMaxX, &fMaxY, &iXDiv, &iYDiv );

	m_pHash = new A2DHash();
	m_pHash->Init( TMemory_GetGlobalHeap(), &ASpawnedCivManager::SetWrapperStamps, fMinX, fMinY, fMaxX, fMaxY, iXDiv, iYDiv, 200, 0x800, 0x400 );

	return TTRUE;
}

// $deBlob: FUNCTION 00763390
ACivWrapper* ASpawnedCivManager::AcquireWrapper( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rFacing, TINT a_iParam )
{
	Toshi::T2GenericDList::Node* pFree = &m_aCivLists[ 3 ];

	if ( pFree->Next() == pFree )
		return TNULL;

	// The last free wrapper
	Toshi::T2GenericDList::Node* pNode = pFree->Prev();
	pNode->Remove();

	ACivWrapper* pWrapper = static_cast<ACivWrapper*>( GetItemOfNode( pNode ) );

	pWrapper->m_iParam = a_iParam;

	// With an ACiv of the pool when one is free, otherwise the wrapper is a point of radius 2
	APooledEntity* pPooled = APooledEntityManager::GetSingleton()->Acquire( 0, a_rPosition, a_rFacing, a_iParam );

	if ( pPooled == TNULL )
	{
		pWrapper->m_vPosition = a_rPosition;
		pWrapper->m_vFacing   = a_rFacing;
		pWrapper->m_fRadius   = 2.0f;
	}
	else
	{
		ABaseEntity* pEntity = pPooled->m_pEntity;

		pWrapper->m_pPooledEntity = pPooled;
		pWrapper->m_vPosition     = pEntity->GetTranslation();
		pWrapper->m_vFacing       = pEntity->GetMatrix().AsBasisVector4( Toshi::BASISVECTOR_FORWARD );
		pWrapper->m_vFacing.w     = 1.0f;
		pWrapper->m_fRadius       = pEntity->GetBoundingSphereRadius();
	}

	m_pHash->Insert( pWrapper );

	pWrapper->m_uiColour                 = 0;
	pWrapper->m_RespawnData.pWaypointSet = TNULL;

	// The wrappers in use are in list 0
	pWrapper->m_Node.InsertBefore( &m_aCivLists[ 0 ] );

	return pWrapper;
}

// $deBlob: FUNCTION 00763490
TBOOL ASpawnedCivManager::ReleaseWrapper( ACivWrapper* a_pWrapper )
{
	m_pHash->Remove( a_pWrapper );

	if ( a_pWrapper->m_pPooledEntity != TNULL )
	{
		APooledEntityManager::GetSingleton()->Release( a_pWrapper->m_pPooledEntity );
		a_pWrapper->m_pPooledEntity = TNULL;
	}

	a_pWrapper->m_Node.Remove();
	a_pWrapper->m_Node.InsertBefore( &m_aCivLists[ 3 ] );

	return TFALSE;
}

// $deBlob: FUNCTION 00762b60
void ASpawnedCivManager::ReleaseAll()
{
	// Retail empties the lists in this order: 0, then 2, then 1 (each from its tail)
	static const TINT s_aiOrder[ 3 ] = { 0, 2, 1 };

	for ( TINT i = 0; i < 3; i++ )
	{
		Toshi::T2GenericDList::Node* pSentinel = &m_aCivLists[ s_aiOrder[ i ] ];

		while ( pSentinel->Prev() != pSentinel )
			ReleaseWrapper( static_cast<ACivWrapper*>( GetItemOfNode( pSentinel->Prev() ) ) );
	}
}

// $deBlob: FUNCTION 00762d50
void ASpawnedCivManager::Update( TFLOAT a_fDeltaTime )
{
	Toshi::T2GenericDList::Node* pInUse = &m_aCivLists[ 0 ];

	// The wrappers of list 0 (the ones in view last frame) are moved out of it, the queries below put back the ones in view
	Toshi::T2GenericDList::Node oLeaving;

	if ( pInUse->Next() != pInUse )
		Toshi::T2GenericDList::Splice( &oLeaving, pInUse->Next(), pInUse->Prev() );

	// Every used viewport: the wrappers near its camera (within 150 units, in front of it) go to the tail of list 0
	AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();

	for ( TINT i = 0; i < AXYZViewportManager::VIEWPORT_Count; i++ )
	{
		AXYZViewportManager::VIEWPORT eViewport = static_cast<AXYZViewportManager::VIEWPORT>( i );

		if ( !pViewportManager->IsViewportUsed( eViewport ) )
			continue;

		ACamera* pCamera = pViewportManager->GetCamera( eViewport );

		if ( pCamera == TNULL )
			continue;

		const Toshi::TMatrix44& mCamera = pCamera->GetMatrix();

		TINT             iNumResults = 0;
		A2DHash::Result* paResults   = m_pHash->Query(
            Toshi::TVector4( mCamera.m_f41, mCamera.m_f42, mCamera.m_f43, mCamera.m_f44 ),
            Toshi::TVector4( mCamera.m_f31, mCamera.m_f32, mCamera.m_f33, 1.0f ),
            150.0f, &iNumResults, 200
        );

		for ( TINT j = 0; j < iNumResults; j++ )
		{
			A2DHashItem* pItem = paResults[ j ].m_pItem;

			if ( pItem != TNULL )
			{
				pItem->m_Node.Remove();
				pItem->m_Node.InsertBefore( pInUse );
			}
		}
	}

	// The wrappers that left the view: an exploding ACiv goes to list 1 and explodes, the others give their ACiv back
	// to the pool and take its place (the ones without an ACiv go to list 2 or 1 by their flags)
	while ( oLeaving.Next() != &oLeaving )
	{
		Toshi::T2GenericDList::Node* pNode = oLeaving.Next();
		pNode->Remove();

		ACivWrapper* pWrapper = static_cast<ACivWrapper*>( GetItemOfNode( pNode ) );

		if ( pWrapper->m_pPooledEntity != TNULL )
		{
			ACiv* pCiv = TSTATICCAST( ACiv, pWrapper->m_pPooledEntity->m_pEntity );

			if ( pCiv->IsExploding() )
			{
				pNode->InsertBefore( &m_aCivLists[ 1 ] );
				pCiv->Explode();
				continue;
			}

			const Toshi::TMatrix44& mCiv = pCiv->GetMatrix();

			pWrapper->m_vPosition = Toshi::TVector4( mCiv.m_f41, mCiv.m_f42, mCiv.m_f43, mCiv.m_f44 );
			pWrapper->m_fRadius   = pCiv->GetBoundingSphereRadius();
			pWrapper->m_vFacing   = Toshi::TVector4( mCiv.m_f31, mCiv.m_f32, mCiv.m_f33, 1.0f );

			m_pHash->Insert( pWrapper );

			pWrapper->m_uiColour = pCiv->GetColour();

			pCiv->SaveRespawnData( &pWrapper->m_RespawnData );

			APooledEntityManager::GetSingleton()->Release( pWrapper->m_pPooledEntity );
			pWrapper->m_pPooledEntity = TNULL;
		}

		pNode->InsertBefore( &m_aCivLists[ ( pWrapper->m_RespawnData.uiFlags & 1 ) != 0 ? 2 : 1 ] );
	}

	// List 0: the wrappers in view get an ACiv from the pool (5 a frame at most, while the pool has one free), or are
	// updated from the ACiv they have
	TINT iNumSpawned = 0;

	for ( Toshi::T2GenericDList::Node* pNode = pInUse->Next(); pNode != pInUse; pNode = pNode->Next() )
	{
		ACivWrapper* pWrapper = static_cast<ACivWrapper*>( GetItemOfNode( pNode ) );

		if ( pWrapper->m_pPooledEntity != TNULL )
		{
			const Toshi::TMatrix44& mCiv = pWrapper->m_pPooledEntity->m_pEntity->GetMatrix();

			if ( mCiv.m_f41 != pWrapper->m_vPosition.x || mCiv.m_f42 != pWrapper->m_vPosition.y || mCiv.m_f43 != pWrapper->m_vPosition.z )
			{
				pWrapper->m_vPosition = Toshi::TVector4( mCiv.m_f41, mCiv.m_f42, mCiv.m_f43, mCiv.m_f44 );
				pWrapper->m_fRadius   = pWrapper->m_pPooledEntity->m_pEntity->GetBoundingSphereRadius();
				pWrapper->m_vFacing   = Toshi::TVector4( mCiv.m_f31, mCiv.m_f32, mCiv.m_f33, 1.0f );

				m_pHash->Insert( pWrapper );
			}
		}
		else if ( iNumSpawned < 5 && APooledEntityManager::GetSingleton()->HasFree( 0 ) )
		{
			Toshi::TVector4 vFacing = pWrapper->m_vFacing;
			vFacing.Normalise();

			APooledEntity* pPooled = APooledEntityManager::GetSingleton()->Acquire( 0, *pWrapper->GetPosition(), vFacing, pWrapper->m_iParam );

			TASSERT( pPooled != TNULL );

			pWrapper->m_pPooledEntity = pPooled;

			ACiv* pCiv = TSTATICCAST( ACiv, pPooled->m_pEntity );

			pCiv->SetTypeAndReset( 2 );

			if ( pWrapper->m_uiColour != 0 )
			{
				pCiv->Liberate( pWrapper->m_uiColour, TTRUE );
				pCiv->SetColour( pWrapper->m_uiColour );
				pCiv->ResumeWalking();
			}
			else
			{
				pCiv->SetState( 1 );
			}

			// The pose controller's byte at 0x2C is set (the name of its class is unknown)
			TUINT8* pPoseController = TREINTERPRETCAST( TUINT8*, pCiv->GetPoseController() );
			pPoseController[ 0x2C ] = 1;

			pCiv->SetActive( TTRUE );
			pCiv->SetVisible( TTRUE );

			if ( pWrapper->m_RespawnData.pWaypointSet != TNULL )
				pCiv->SetWaypointSet( -1, pWrapper->m_RespawnData.iNextPoint, &pWrapper->m_vPosition, &pWrapper->m_RespawnData );

			iNumSpawned++;
		}
		else if ( ( pWrapper->m_RespawnData.uiFlags & 1 ) != 0 )
		{
			APropPlatform::UpdateRespawnRoute( a_fDeltaTime, &pWrapper->m_RespawnData, &pWrapper->m_vPosition );

			m_pHash->Insert( pWrapper );
		}
	}

	// List 2: the wrappers that are updated, and the ones whose flag is clear move to list 1
	Toshi::T2GenericDList::Node* pWaiting = &m_aCivLists[ 2 ];

	for ( Toshi::T2GenericDList::Node* pNode = pWaiting->Next(); pNode != pWaiting; )
	{
		Toshi::T2GenericDList::Node* pNext    = pNode->Next();
		ACivWrapper*                 pWrapper = static_cast<ACivWrapper*>( GetItemOfNode( pNode ) );

		APropPlatform::UpdateRespawnRoute( a_fDeltaTime, &pWrapper->m_RespawnData, &pWrapper->m_vPosition );

		m_pHash->Insert( pWrapper );

		if ( ( pWrapper->m_RespawnData.uiFlags & 1 ) == 0 )
		{
			pNode->Remove();
			pNode->InsertBefore( &m_aCivLists[ 1 ] );
		}

		pNode = pNext;
	}
}

// $deBlob: FUNCTION 00763500
void ASpawnedCivManager::SetWrapperStamps( TUINT32 a_uiGeneration )
{
	ASpawnedCivManager* pManager = GetSingleton();

	for ( TINT i = 0; i < 3; i++ )
	{
		Toshi::T2GenericDList::Node* pSentinel = &pManager->m_aCivLists[ i ];

		for ( Toshi::T2GenericDList::Node* pNode = pSentinel->Next(); pNode != pSentinel; pNode = pNode->Next() )
			GetItemOfNode( pNode )->m_uiStamp = a_uiGeneration;
	}
}
