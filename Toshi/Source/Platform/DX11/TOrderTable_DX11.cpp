#include "ToshiPCH.h"
#include "Render/TOrderTable.h"
#include "Render/TShader.h"
#include "TRender_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TUINT          TOrderTable::s_uiNumRegisteredMaterials = 0;
TUINT          TOrderTable::s_uiMaxMaterials           = 0;
TRegMaterial*  TOrderTable::s_pRegMaterials            = TNULL;
TUINT          TOrderTable::s_uiMaxRenderPackets       = 0;
TUINT          TOrderTable::s_uiNumRenderPackets       = 0;
TRenderPacket* TOrderTable::s_pRenderPackets           = TNULL;
TUINT          TOrderTable::s_uiOrigMaxRenderPackets   = 0;
// $deBlob: FUNCTION 00858f50
TNodeList<TRegMaterial> TOrderTable::s_llRegMatRegisteredList;
// $deBlob: FUNCTION 00858f00
TNodeList<TRegMaterial> TOrderTable::s_llRegMatFreeList;

// $deBlob: FUNCTION 006adf10
TBOOL TOrderTable::CreateStaticData( TUINT a_uiMaxMaterials, TUINT a_uiMaxRenderPackets )
{
	s_uiMaxRenderPackets       = a_uiMaxRenderPackets;
	s_uiNumRenderPackets       = 0;
	s_uiNumRegisteredMaterials = 0;
	s_pRenderPackets           = TNULL;
	s_pRenderPackets           = TSTATICCAST( TRenderPacket, TMemalign( 32, a_uiMaxRenderPackets * sizeof( TRenderPacket ) ) );
	s_uiOrigMaxRenderPackets   = s_uiMaxRenderPackets;

	s_uiMaxMaterials = a_uiMaxMaterials;
	s_pRegMaterials  = TNULL;
	s_pRegMaterials  = new TRegMaterial[ a_uiMaxMaterials ];

	for ( TUINT i = 0; i < s_uiMaxMaterials; i++ )
	{
		s_pRegMaterials[ i ].SetOrderTable( TNULL );
		s_llRegMatFreeList.InsertHead( &s_pRegMaterials[ i ] );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006ae090
TBOOL TOrderTable::Create( TShader* a_pShader, TINT a_iPriority, TUINT8 a_uiIndex )
{
	m_pShader     = a_pShader;
	m_pLastRegMat = TNULL;
	TPriList<TOrderTable>* pOrderTables = &TRenderDX11::Interface()->GetOrderTables();
	SetPriority( a_iPriority );
	pOrderTables->Insert( this );
	m_uiIndex = a_uiIndex;

	return TTRUE;
}

// $deBlob: FUNCTION 006ae0d0
TOrderTable::TOrderTable()
{
	m_pShader                = TNULL;
	m_fnPreFlushCallback     = TNULL;
	m_pPreFlushCallbackData  = TNULL;
	m_fnPostFlushCallback    = TNULL;
	m_pPostFlushCallbackData = TNULL;
	m_pLastRegMat            = TNULL;
}

// $deBlob: FUNCTION 006ae110
TOrderTable::~TOrderTable()
{
	// NOTE: retail doesn't remove the order table from TRenderDX11's list
	if ( s_pRegMaterials )
	{
		DeregisterAllMaterials();
		s_llRegMatFreeList.RemoveAll();

		delete[] s_pRegMaterials;
		s_pRegMaterials = TNULL;
	}

	if ( s_pRenderPackets )
	{
		TFree( s_pRenderPackets );
		s_pRenderPackets = TNULL;
	}
}

// $deBlob: FUNCTION 006ae160
TRegMaterial* TOrderTable::RegisterMaterial( TMaterial* a_pMat )
{
	TASSERT( s_uiNumRenderPackets == 0 );

	if ( a_pMat->GetRegMaterial() )
		return a_pMat->GetRegMaterial();

	TASSERT( TFALSE == s_llRegMatFreeList.IsEmpty() );

	if ( s_llRegMatFreeList.IsEmpty() )
		return TNULL;

	TRegMaterial* pRegMat = s_llRegMatFreeList.Remove( s_llRegMatFreeList.Head() );
	s_llRegMatRegisteredList.InsertTail( pRegMat );
	s_uiNumRegisteredMaterials++;

	pRegMat->SetFlags( TRegMaterial::State_Registered );
	pRegMat->SetMaterial( a_pMat );
	pRegMat->SetOrderTable( this );
	a_pMat->SetRegMaterial( pRegMat );

	return pRegMat;
}

// $deBlob: FUNCTION 006ae230
void TOrderTable::DeregisterMaterial( TRegMaterial* a_pRegMat )
{
	if ( a_pRegMat && HASANYFLAG( a_pRegMat->GetFlags(), TRegMaterial::State_Registered ) )
	{
		TMaterial* pMaterial = a_pRegMat->GetMaterial();
		TASSERT( s_uiNumRenderPackets == 0 );

		s_llRegMatRegisteredList.Remove( a_pRegMat );
		s_llRegMatFreeList.InsertHead( a_pRegMat );
		s_uiNumRegisteredMaterials--;

		a_pRegMat->SetFlags( 0 );
		a_pRegMat->SetMaterial( TNULL );
		a_pRegMat->SetOrderTable( TNULL );
		pMaterial->SetRegMaterial( TNULL );
	}
}

// $deBlob: FUNCTION 006ae2c0
void TOrderTable::DeregisterAllMaterials()
{
	// NOTE: DeregisterMaterial inlined; retail would spin forever on an unregistered material in the list (never
	// happens: only RegisterMaterial adds to it)
	while ( !s_llRegMatRegisteredList.IsEmpty() )
		DeregisterMaterial( s_llRegMatRegisteredList.Head() );
}

// $deBlob: FUNCTION 006ae3e0
TRenderPacket* TOrderTable::AllocRenderPacket()
{
	TRenderPacket* pPacket = &s_pRenderPackets[ s_uiNumRenderPackets++ ];

	// NOTE: retail returns TNULL for the last packet as well
	return ( s_uiNumRenderPackets < s_uiMaxRenderPackets ) ? pPacket : TNULL;
}

// $deBlob: FUNCTION 006ae410
void TOrderTable::UseMaterial( TRegMaterial* a_pRegMat )
{
	if ( !HASANYFLAG( a_pRegMat->GetFlags(), TRegMaterial::State_Used ) )
	{
		a_pRegMat->SetFlags( a_pRegMat->GetFlags() | TRegMaterial::State_Used );
		a_pRegMat->SetNextUsedMaterial( m_pLastRegMat );
		m_pLastRegMat = a_pRegMat;
	}
}

// $deBlob: FUNCTION 006ae430
void TOrderTable::Render()
{
	for ( TRegMaterial* pRegMat = m_pLastRegMat; pRegMat != TNULL; pRegMat = pRegMat->GetNextRegMat() )
		pRegMat->Render();

	s_uiNumRenderPackets = 0;
	m_pLastRegMat        = TNULL;
}

// $deBlob: FUNCTION 006ae4a0
void TOrderTable::Flush()
{
	if ( m_fnPreFlushCallback )
		m_fnPreFlushCallback( this, m_pPreFlushCallbackData );

	if ( m_pLastRegMat )
	{
		m_pShader->StartFlush();

		for ( TRegMaterial* pRegMat = m_pLastRegMat; pRegMat != TNULL; pRegMat = pRegMat->GetNextRegMat() )
			pRegMat->Render();

		m_pShader->EndFlush();
	}

	if ( m_fnPostFlushCallback )
		m_fnPostFlushCallback( this, m_pPostFlushCallbackData );

	s_uiNumRenderPackets = 0;
	m_pLastRegMat        = TNULL;
}

// $deBlob: FUNCTION 006ae530
void* TOrderTable::GetRenderPacketMem( TUINT a_uiSize )
{
	TUINT uiNumRenderPacketsToLeave = s_uiMaxRenderPackets - ( a_uiSize + sizeof( TRenderPacket ) - 1 ) / sizeof( TRenderPacket );

	// NOTE: unused in de Blob, it asserts as soon as it's called
	TASSERT( TFALSE );
	TASSERT( s_uiNumRenderPackets <= uiNumRenderPacketsToLeave );
	TASSERT( s_uiMaxRenderPackets == s_uiOrigMaxRenderPackets );
	TASSERT( uiNumRenderPacketsToLeave < s_uiMaxRenderPackets );
	TASSERT( TINT( uiNumRenderPacketsToLeave ) >= 0 );

	s_uiMaxRenderPackets = uiNumRenderPacketsToLeave;
	return &s_pRenderPackets[ uiNumRenderPacketsToLeave ];
}

// $deBlob: FUNCTION 006ae670
void TOrderTable::RestoreRenderPacketMem()
{
	s_uiMaxRenderPackets = s_uiOrigMaxRenderPackets;
}

// $deBlob: FUNCTION 006ae680
void TOrderTable::SetPreFlushCallback( t_PreFlushCallback a_fnCallback, void* a_pCustomData )
{
	m_fnPreFlushCallback    = a_fnCallback;
	m_pPreFlushCallbackData = a_pCustomData;
}

// $deBlob: FUNCTION 006ae6a0
void TOrderTable::SetPostFlushCallback( t_PostFlushCallback a_fnCallback, void* a_pCustomData )
{
	m_fnPostFlushCallback    = a_fnCallback;
	m_pPostFlushCallbackData = a_pCustomData;
}

// $deBlob: FUNCTION 006ae770
TRenderPacket* TRegMaterial::AddRenderPacket( TMesh* a_pMesh )
{
	TRenderPacket* pPreviousPacket = m_pLastRenderPacket;

	// NOTE: retail doesn't check for TNULL (out of render packets)
	m_pLastRenderPacket = TOrderTable::AllocRenderPacket();
	m_pLastRenderPacket->SetNextPacket( pPreviousPacket );

	m_pOrderTable->UseMaterial( this );
	m_pLastRenderPacket->SetMesh( a_pMesh );
	m_pLastRenderPacket->SetMaterial( a_pMesh->GetMaterial() );

	return m_pLastRenderPacket;
}

// $deBlob: FUNCTION 006ae7f0
void TRegMaterial::Render()
{
	m_pMaterial->PreRender();

	for ( TRenderPacket* pPacket = m_pLastRenderPacket; pPacket != TNULL; pPacket = pPacket->GetNextPacket() )
		pPacket->GetMesh()->GetOwnerShader()->Render( pPacket );

	m_State &= ~State_Used;
	m_pLastRenderPacket = TNULL;
	m_pMaterial->PostRender();
}

TOSHI_NAMESPACE_END
