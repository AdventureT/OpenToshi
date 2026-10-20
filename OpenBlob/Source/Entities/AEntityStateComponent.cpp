#include "pch.h"
#include "AEntityStateComponent.h"
#include "AEntityData.h"
#include "AEntityFactory.h"
#include "ABaseEntity.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007c6220
AEntityStateComponent::AEntityStateComponent( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	AEntityTypeData* pTypeData = a_pEntity->GetTypeData();

	TCHAR szSymbol[ 1024 ];
	T2String8::Format( szSymbol, "%s_StateData_Main", AEntityData::GetDirectoryName( pTypeData->m_strName.GetString() ) );

	m_pHeader = TSTATICCAST( Header, AEntityData::GetSingleton()->GetStateData( szSymbol ) );
}

// $deBlob: FUNCTION 007c6310
TFLOAT AEntityStateComponent::GetStateLength( TINT a_iState )
{
	Header* pHeader = m_pHeader;
	TASSERT( a_iState >= 0 && a_iState < pHeader->iDataCount );

	return pHeader->pData[ a_iState ].fLength;
}

// $deBlob: FUNCTION 007c6360
TFLOAT AEntityStateComponent::GetStateAnimBlendTime( TINT a_iCurState, TINT a_iNewState )
{
	Header* pHeader = m_pHeader;

	if ( a_iCurState == -1 )
		return pHeader->pData[ 0 ].fDefaultBlendTime;

	TASSERT( a_iCurState >= 0 && a_iCurState < pHeader->iDataCount );

	StateData& rState = pHeader->pData[ a_iCurState ];

	for ( TINT i = 0; i < rState.iNumBlendTimes; i++ )
	{
		if ( rState.pBlendTimes[ i ].iNewState == a_iNewState )
			return rState.pBlendTimes[ i ].fTime;
	}

	return rState.fDefaultBlendTime;
}

// $deBlob: FUNCTION 007c63f0
// $deBlob: FUNCTION 007c6300
AEntityStateComponent::~AEntityStateComponent()
{
}
