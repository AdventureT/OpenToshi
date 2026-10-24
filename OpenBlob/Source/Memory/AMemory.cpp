#include "pch.h"
#include "AMemory.h"
#include <XUI/TXUI.h>
#include <Render/TAssetInit.h>
#include <Render/TAnimation.h>
#include <Render/TModel.h>
#include <Shaders/WorldShader/TWorldShader.h>
#include "Platform/DX11/ASkinShader_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TMemoryHeap* AMemory::ms_pTRBHeap;
TMemoryHeap* AMemory::ms_pUnkHeap;

AMemory::PoolDefinitionInfo AMemory::ms_aPoolDefinitionInfo[ POOL_NumOf ] = {
	{ "POOL_FrequentAllocations", POOL_FrequentAllocations, 0x300000 },
	{ "POOL_XUI", POOL_XUI, 0x1400000 },
};

TMemoryHeap*      AMemory::ms_apMemoryBlocks[ POOL_NumOf ];
T2MemoryAllocator AMemory::ms_aAllocators[ POOL_NumOf ];

// $deBlob: FUNCTION 005e56e0
void AMemory::CreatePools()
{
	for ( TINT i = 0; i < POOL_NumOf; i++ )
		CreatePool( POOL( i ) );
}

// $deBlob: FUNCTION 005e5860
void AMemory::DestroyPools()
{
	for ( TINT i = 0; i < POOL_NumOf; i++ )
		DestroyPool( POOL( i ) );
}

// $deBlob: FUNCTION 005e5980
void AMemory::CreatePool( POOL a_eType )
{
	TASSERT( a_eType >= AMemory::POOL_FrequentAllocations );
	TASSERT( a_eType < AMemory::POOL_NumOf );
	TASSERT( TFALSE == IsPoolCreated( a_eType ) );

	if ( ms_aPoolDefinitionInfo[ a_eType ].m_eParent != a_eType )
	{
		TASSERT( ms_aPoolDefinitionInfo[ a_eType ].m_eParent >= 0 );
		TASSERT( ms_aPoolDefinitionInfo[ a_eType ].m_eParent < AMemory::POOL_NumOf );
	}

	// NOTE: the parent pool isn't used, every pool is allocated from the global heap
	TSIZE uiSize = ms_aPoolDefinitionInfo[ a_eType ].m_uiSize;
	void* pMem   = TMemoryHeap_Malloc( TMemory_GetGlobalHeap(), uiSize );

	TMemoryHeap* pHeap = TMemory_CreateHeapInPlace( pMem, uiSize, TMEMORYHEAP_USEMUTEX, ms_aPoolDefinitionInfo[ a_eType ].m_szName );

	ms_apMemoryBlocks[ a_eType ] = pHeap;
	ms_aAllocators[ a_eType ].SetHeap( pHeap );
}

// $deBlob: FUNCTION 005e5b10
void AMemory::DestroyPool( POOL a_eType )
{
	TASSERT( a_eType >= AMemory::POOL_FrequentAllocations );
	TASSERT( a_eType < AMemory::POOL_NumOf );
	TASSERT( IsPoolCreated( a_eType ) );

	TMemory_DestroyHeap( ms_apMemoryBlocks[ a_eType ] );
	TFree( ms_apMemoryBlocks[ a_eType ] );

	ms_apMemoryBlocks[ a_eType ] = TNULL;
	ms_aAllocators[ a_eType ].SetHeap( TNULL );
}

// $deBlob: FUNCTION 005e5c20
void AMemory::FlushPool( POOL a_eType )
{
	TASSERT( a_eType >= AMemory::POOL_FrequentAllocations && a_eType < AMemory::POOL_NumOf );
}

// $deBlob: FUNCTION 005e5c70
TBOOL AMemory::IsPoolCreated( POOL a_eType )
{
	TASSERT( a_eType < AMemory::POOL_NumOf );
	return ms_apMemoryBlocks[ a_eType ] != TNULL;
}

// $deBlob: FUNCTION 005e5ce0
void AMemory::CheckAllPoolsDestroyed()
{
	TINT iNumPools = 0;

	for ( TINT i = 0; i < POOL_NumOf; i++ )
	{
		if ( ms_apMemoryBlocks[ i ] != TNULL )
			iNumPools++;
	}

	if ( iNumPools > 0 )
	{
		TASSERT( !"Expected no memory pools remaining" );
	}
}

// $deBlob: FUNCTION 005e5d70
void __stdcall AMemory::Console_DumpMemoryStats( TString8* a_pArgs, TINT a_iNumArgs )
{
}

// $deBlob: FUNCTION 005e5d80
void __stdcall AMemory::Console_DumpMemoryStatsSimple( TString8* a_pArgs, TINT a_iNumArgs )
{
}

// $deBlob: FUNCTION 005e5d90
void __stdcall AMemory::Console_MarkMemoryPoint( TString8* a_pArgs, TINT a_iNumArgs )
{
}

// $deBlob: FUNCTION 005e5da0
void __stdcall AMemory::Console_ShowMemoryPointDelta( TString8* a_pArgs, TINT a_iNumArgs )
{
}

// $deBlob: FUNCTION 005e5db0
void AMemory::GetGlobalHeapUsage( TUINT* a_puiUsed, TUINT* a_puiUnk )
{
	TMemoryHeapInfo oInfo;
	TMemoryHeap_GetInfo( TMemory_GetGlobalHeap(), &oInfo );

	*a_puiUsed = oInfo.uiUsed;
	*a_puiUnk  = 0;
}

// $deBlob: FUNCTION 005e5e00
void AMemory::SetupTRBAllocators()
{
	TTRB::SetDefaultMemoryFunctions( TemporaryTRBAllocator, TemporaryTRBDeallocator, TNULL );
	ms_pTRBHeap = TMemory_GetGlobalHeap();
}

// $deBlob: FUNCTION 005e5e20
TMemoryHeap* AMemory::SetTRBHeap( TMemoryHeap* a_pHeap )
{
	TMemoryHeap* pOldHeap = ms_pTRBHeap;
	ms_pTRBHeap           = a_pHeap;
	return pOldHeap;
}

// $deBlob: FUNCTION 005e5e40
void* __stdcall AMemory::TemporaryTRBAllocator( TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
	return TMemoryHeap_Memalign( ms_pTRBHeap, a_iAlignment, a_uiSize );
}

// $deBlob: FUNCTION 005e5e60
void __stdcall AMemory::TemporaryTRBDeallocator( TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData )
{
	TFree( a_pMem );
}

// $deBlob: FUNCTION 005e5e70
void* __stdcall AMemory::XUITRBAllocator( TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
	return TMemoryHeap_Memalign( TXUI::ms_pXUIMemoryBlock, a_iAlignment, a_uiSize );
}

// $deBlob: FUNCTION 005e5e90
void __stdcall AMemory::XUITRBDeallocator( TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData )
{
	TFree( a_pMem );
}

// $deBlob: FUNCTION 005e5ea0
void AMemory::SetAssetHeaps( TMemoryHeap* a_pHeap, TMemoryHeap** a_apOldHeaps )
{
	a_apOldHeaps[ 0 ] = TAssetInit::g_pMemHeap;
	a_apOldHeaps[ 1 ] = GetTAnimMemoryHeap();
	a_apOldHeaps[ 2 ] = TModel::GetMemHeap();
	a_apOldHeaps[ 3 ] = TWorldShader::GetWorldMemHeap();
	a_apOldHeaps[ 4 ] = ASkinShaderHAL::ms_pMemHeap;
	a_apOldHeaps[ 5 ] = ms_pUnkHeap;

	TAssetInit::g_pMemHeap = a_pHeap;
	SetTAnimMemoryHeap( a_pHeap );
	TModel::SetMemHeap( a_pHeap );
	TWorldShader::SetWorldMemHeap( a_pHeap );
	ASkinShaderHAL::ms_pMemHeap = a_pHeap;
	ms_pUnkHeap                 = a_pHeap;
}

// $deBlob: FUNCTION 005e5f10
void AMemory::RestoreAssetHeaps( TMemoryHeap** a_apHeaps )
{
	TAssetInit::g_pMemHeap = a_apHeaps[ 0 ];
	SetTAnimMemoryHeap( a_apHeaps[ 1 ] );
	TModel::SetMemHeap( a_apHeaps[ 2 ] );
	TWorldShader::SetWorldMemHeap( a_apHeaps[ 3 ] );
	ASkinShaderHAL::ms_pMemHeap = a_apHeaps[ 4 ];
	ms_pUnkHeap                 = a_apHeaps[ 5 ];
}
