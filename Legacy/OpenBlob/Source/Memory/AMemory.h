#pragma once
#include "Toshi/Memory/TMemory.h"

class AMemory
{
public:
	typedef TUINT32 Pool;
	enum POOL_ : Pool
	{
		POOL_FrequentAllocations,
		POOL_XUI,
		POOL_NumOf
	};

	struct PoolDefinitionInfo
	{
		const TCHAR* Name;
		Pool         Type;
		TUINT32      Size;
	};

public:
	AMemory() = delete;

	static void CreatePools();

	static void CreatePool(Pool type);
	static void DestroyPool(Pool type);

	static TBOOL IsPoolCreated(Pool type)
	{
		TASSERT(type < POOL_NumOf);
		return ms_apMemoryBlocks[type] != TNULL;
	}

	static Toshi::TMemoryHeap* GetPool(Pool type)
	{
		return ms_apMemoryBlocks[type];
	}

public:
	static PoolDefinitionInfo  ms_aPoolDefinitionInfo[POOL_NumOf];
	static Toshi::TMemoryHeap* ms_apMemoryBlocks[POOL_NumOf];
};
