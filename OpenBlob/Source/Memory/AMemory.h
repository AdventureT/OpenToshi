#pragma once
#include <Toshi/T2MemoryAllocator.h>
#include <File/TTRB.h>
#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Purpose: the game's memory pools. Each pool is a heap allocated from the global heap, with an allocator (used by the
// strings, see AppBoot.cpp).
//-----------------------------------------------------------------------------
class AMemory
{
public:
	enum POOL
	{
		POOL_FrequentAllocations,
		POOL_XUI,
		POOL_NumOf
	};

	struct PoolDefinitionInfo
	{
		const TCHAR* m_szName;  // 0x00
		POOL         m_eParent; // 0x04
		TSIZE        m_uiSize;  // 0x08
	};

public:
	AMemory() = delete;

	static void CreatePools();
	static void DestroyPools();

	static void CreatePool( POOL a_eType );
	static void DestroyPool( POOL a_eType );
	static void FlushPool( POOL a_eType );

	static TBOOL IsPoolCreated( POOL a_eType );
	static void  CheckAllPoolsDestroyed();

	// The console commands mem_show_stats, mem_show_stats_simple, mem_mark_point and mem_show_point_delta (AConsole);
	// empty in de Blob
	static void __stdcall Console_DumpMemoryStats( Toshi::TString8* a_pArgs, TINT a_iNumArgs );
	static void __stdcall Console_DumpMemoryStatsSimple( Toshi::TString8* a_pArgs, TINT a_iNumArgs );
	static void __stdcall Console_MarkMemoryPoint( Toshi::TString8* a_pArgs, TINT a_iNumArgs );
	static void __stdcall Console_ShowMemoryPointDelta( Toshi::TString8* a_pArgs, TINT a_iNumArgs );

	// Sets the default memory functions of the TRBs
	static void SetupTRBAllocators();

	// Sets the heap of TemporaryTRBAllocator, returns the previous one (AXUIState loads its TRBs in the XUI heap)
	static TMemoryHeap* SetTRBHeap( TMemoryHeap* a_pHeap );

	// The used size of the global heap (a_puiUnk gets 0)
	static void GetGlobalHeapUsage( TUINT* a_puiUsed, TUINT* a_puiUnk );
	// Sets the heaps of the assets (the models, the animations, the model instances, the world and skin shaders and
	// ms_pUnkHeap) to a_pHeap; the previous ones go to a_apOldHeaps (6), RestoreAssetHeaps sets them back
	static void SetAssetHeaps( TMemoryHeap* a_pHeap, TMemoryHeap** a_apOldHeaps );
	static void RestoreAssetHeaps( TMemoryHeap** a_apHeaps );

	static TMemoryHeap*              GetPool( POOL a_eType ) { return ms_apMemoryBlocks[ a_eType ]; }
	static Toshi::T2MemoryAllocator* GetAllocator( POOL a_eType ) { return &ms_aAllocators[ a_eType ]; }

	// The memory functions SetupTRBAllocators sets (AXUIState sets them on its TRBs too)
	static void* __stdcall TemporaryTRBAllocator( Toshi::TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );
	static void __stdcall TemporaryTRBDeallocator( Toshi::TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData );

	// The TRB allocator of the XUI heap (TXUI::ms_pXUIMemoryBlock; the full screen help's TRB)
	static void* __stdcall XUITRBAllocator( Toshi::TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData );
	static void __stdcall XUITRBDeallocator( Toshi::TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData );

public:
	static PoolDefinitionInfo       ms_aPoolDefinitionInfo[ POOL_NumOf ];
	static TMemoryHeap*             ms_apMemoryBlocks[ POOL_NumOf ];
	static Toshi::T2MemoryAllocator ms_aAllocators[ POOL_NumOf ];
	static TMemoryHeap*             ms_pTRBHeap;
	static TMemoryHeap*             ms_pUnkHeap; // only set by the heap setters (no reader)
};
