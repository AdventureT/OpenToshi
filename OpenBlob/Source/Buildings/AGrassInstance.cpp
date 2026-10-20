#include "pch.h"
#include "AGrassInstance.h"
#include "ABuildingManager.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Platform/DX11/AJellyShader_DX11.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0057b8b0
void AGrassInstance::OnCreated( TerrainTypes::CellInstance* a_pInstance )
{
	AGrassInstance* pGrass     = TSTATICCAST( AGrassInstance, a_pInstance->m_pModelInstance2 );
	pGrass->m_pTerrainInstance = a_pInstance;

	ABuildingManager::GetSingleton()->AddBuilding( pGrass );
}

// $deBlob: FUNCTION 0057b8d0
AGrassInstance::AGrassInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints )
    : ABuildingInstance( a_pModel, a_bCreateSoundBreakpoints )
{
	m_bPainted  = TFALSE;
	m_bPainting = TFALSE;
}

// $deBlob: FUNCTION 0057ba40
// $deBlob: FUNCTION 0057b900
AGrassInstance::~AGrassInstance()
{
}

// $deBlob: FUNCTION 0057b920
TINT AGrassInstance::Reset()
{
	TINT iResult = ABuildingInstance::Reset();
	m_bPainted   = TFALSE;
	m_bPainting  = TFALSE;

	return iResult;
}

// $deBlob: FUNCTION 0057b940
void AGrassInstance::UpdateImp( TFLOAT a_fDeltaTime )
{
	if ( !m_bPainting )
		return;

	if ( AJellyShader_DX11::GetSingleton()->IsPainting( this ) )
	{
		// Stays in ABuildingManager's active instances until the jelly is done
		m_uiFlags |= Flags_Update;
		return;
	}

	m_bPainted  = TTRUE;
	m_bPainting = TFALSE;
}

// $deBlob: FUNCTION 0057b980
void AGrassInstance::OnPaint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Grass );

	TSTATICCAST( AGrassInstance, a_pInstance->m_pModelInstance2 )->Paint();
}

// $deBlob: FUNCTION 0057b9d0
void AGrassInstance::Paint()
{
	if ( m_bPainted || m_bPainting )
		return;

	ABuildingManager::GetSingleton()->AddActiveInstance( this );
	m_bPainting = TTRUE;

	// NOTE: a function static copy of ABlobColourComponent::ms_auiColours[ COLOUR_WHITE ]
	static TUINT32 s_uiWhite = ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_WHITE ];
	AJellyShader_DX11::GetSingleton()->StartPaint( this, s_uiWhite, s_uiWhite, 3, -1, 1.0f );
}
