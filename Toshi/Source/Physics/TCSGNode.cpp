#include "ToshiPCH.h"
#include "TCSGNode.h"
#include "Havok/TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

T2Vector<TCSGNode*, 200> TCSGNode::ms_oCache;

// $deBlob: FUNCTION 00733860
TCSGNode* TCSGNode::Get( TINT a_eType, TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcTransform )
{
	for ( TINT i = 0; i < ms_oCache.Size(); i++ )
	{
		TCSGNode* pNode = ms_oCache[ i ];

		if ( pNode->m_pShapeData->GetType() != a_eType || pNode->m_pShapeData != a_pShapeData )
			continue;

		if ( !pNode->m_oTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ).IsEqual( a_rcTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) ) )
			continue;

		if ( a_eType != TShapeBaseClass::Type_Sphere )
		{
			if ( !pNode->m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ).IsEqual( a_rcTransform.AsBasisVector4( BASISVECTOR_FORWARD ) ) )
				continue;

			if ( a_eType != TShapeBaseClass::Type_Capsule && a_eType != TShapeBaseClass::Type_Cylinder )
			{
				if ( !pNode->m_oTransform.AsBasisVector4( BASISVECTOR_RIGHT ).IsEqual( a_rcTransform.AsBasisVector4( BASISVECTOR_RIGHT ) ) )
					continue;

				if ( !pNode->m_oTransform.AsBasisVector4( BASISVECTOR_UP ).IsEqual( a_rcTransform.AsBasisVector4( BASISVECTOR_UP ) ) )
					continue;
			}
		}

		pNode->m_iRefCount++;
		return pNode;
	}

	TCSGNode* pNode = new ( TMalloc( sizeof( TCSGNode ), TPhysicsManager::GetHeap() ) ) TCSGNode();
	ms_oCache.PushBack( pNode );

	pNode->m_pShapeData = a_pShapeData;
	pNode->m_oTransform = a_rcTransform;
	pNode->m_iRefCount++;
	return pNode;
}

// $deBlob: FUNCTION 00733a30
void TCSGNode::Release()
{
	if ( --m_iRefCount == 0 )
	{
		ms_oCache.FindAndErase( this );
		delete this;
	}
}

TOSHI_NAMESPACE_END
