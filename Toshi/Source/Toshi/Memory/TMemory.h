#pragma once
#include "Toshi/Core/Core.h"
#include "Toshi2/Thread/T2Mutex.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/TMemoryContext_Win.h"
#endif // TOSHI_SKU_WINDOWS

#define TMEMORY_NEW(TYPE, HEAP, ...) ((HEAP != TNULL) ? new (HEAP) TYPE(__VA_ARGS__) : new TYPE(__VA_ARGS__))

TOSHI_NAMESPACE_START

class TMemoryHeap;

typedef TUINT32 TMemoryHeapFlags;
enum TMemoryHeapFlags_ : TMemoryHeapFlags
{
	TMemoryHeapFlags_UseMutex    = BITFIELD(0),
	TMemoryHeapFlags_AllocAsPile = BITFIELD(2),
};

class TMemory
{
public:
	static constexpr TINT HEAP_MAXNAME = 15;

	typedef TUINT32 Flags;
	typedef TUINT32 Error;
	typedef TUINT32 BlockSize;

	enum Flags_ : Flags
	{
		Flags_Standard      = 0,
		Flags_NativeMethods = BITFIELD(1)
	};

	enum Error_ : Error
	{
		Error_Ok   = 0,
		Error_Heap = 1
	};

public:
	TMemory(Flags flags = Flags_Standard, BlockSize blockSize = 640 * 1024 * 1024)
	    : m_Flags(flags), m_GlobalSize(blockSize) { s_pSingleton = this; }

	static void         dlheapfree(TMemoryHeap* heap, void* mem);
	static void         dlheapdestroy(TMemoryHeap* heap);
	static void*        dlheapmalloc(TMemoryHeap* heap, size_t size);
	static void*        dlheapcalloc(TMemoryHeap* heap, size_t nitems, size_t size);
	static void*        dlheaprealloc(TMemoryHeap* heap, void* mem, size_t newsize);
	static void*        dlheapmemalign(TMemoryHeap* heap, size_t alignment, size_t size);
	static TMemoryHeap* dlheapcreateinplace(void* ptr, size_t heapSize, TMemoryHeapFlags flags, const TCHAR name[HEAP_MAXNAME]);
	static TMemoryHeap* dlheapcreatesubheap(TMemoryHeap* heap, size_t size, TMemoryHeapFlags flags, const TCHAR name[HEAP_MAXNAME]);
	static TMemoryHeap* dlheapcreate(TMemoryHeap* heap, size_t size, TMemoryHeapFlags flags, const TCHAR name[HEAP_MAXNAME]) { return TMemory::dlheapcreatesubheap(heap, size, flags, name); }

	// $deBlob: FUNCTION 006fbb70
	static TMemoryHeap* CreateHeapInPlace(void* ptr, size_t heapSize, TMemoryHeapFlags flags, const TCHAR name[HEAP_MAXNAME]) { return TMemory::dlheapcreateinplace(ptr, heapSize, flags, name); }
	// $deBlob: FUNCTION 006fbb60
	static TMemoryHeap* CreateHeap(size_t size, TMemoryHeapFlags flags, const TCHAR name[HEAP_MAXNAME]) { return TMemory::dlheapcreate(s_GlobalHeap, size, flags, name); }
	static void         DestroyHeap(TMemoryHeap* heap) { TMemory::dlheapdestroy(heap); }

	static void OutOfMem(TMemoryHeap* heap, size_t size);
	static void Shutdown();

	// $deBlob: FUNCTION 006fbe10
	static TMemoryHeap*       GetGlobalHeap() { return s_GlobalHeap; }
	// $deBlob: FUNCTION 006fbfc0
	static void               AcquireMutex() { TMemory::s_GlobalMutex.Lock(); }
	static void               ReleaseMutex() { TMemory::s_GlobalMutex.Unlock(); }
	static unsigned long long GetNumOfAllocatedBytes() { return s_NumAllocatedBytes; }

private:
	static Flags GetFlags() { return s_pSingleton->m_Flags; }

public:
	/*
        * Platform specific methods
        * Define them in TMemory_{Platform}.cpp
        */

	Error Init();

public:
	inline static TMemoryContext s_Context;

private:
	inline static TMemoryHeap*       s_GlobalHeap;
	inline static T2Mutex            s_GlobalMutex;
	inline static TMemory*           s_pSingleton;
	inline static unsigned long long s_NumAllocatedBytes;

private:
	Flags     m_Flags;
	BlockSize m_GlobalSize;
};

class TMemoryHeap
{
public:
	friend class TMemory;

public:
	void*        Malloc(size_t size) { return TMemory::dlheapmalloc(this, size); }
	static void* Malloc(TMemoryHeap* heap, size_t size) { return TMemory::dlheapmalloc(heap, size); }
	void*        Calloc(size_t nitems, size_t size) { return TMemory::dlheapcalloc(this, nitems, size); }
	void*        Realloc(void* mem, size_t newsize) { return TMemory::dlheaprealloc(this, mem, newsize); }
	void*        Memalign(size_t alignment, size_t size) { return TMemory::dlheapmemalign(this, alignment, size); }
	void         Free(void* mem) { TMemory::dlheapfree(this, mem); }

	void SetName(const TCHAR* name)
	{
		strncpy_s(m_Name, name, TMemory::HEAP_MAXNAME);
		m_Name[TMemory::HEAP_MAXNAME] = '\0';
	}

	void* GetMSpace() const { return m_MSpace; }

private:
	// $deBlob: FUNCTION 006fbf80
	void CreateMutex() { m_Mutex.Create(); }
	void DestroyMutex() { m_Mutex.Destroy(); }

	static void* AllocAsPile(TMemoryHeap* heap, size_t size, size_t alignment = 4);

private:
	TMemoryHeapFlags m_Flags;
	T2Mutex          m_Mutex;
	TCHAR*           m_SubHeapBuffer;
	void*            m_MSpace;
	TCHAR*           m_PileData;
	TUINT32          m_PileSize;
	TCHAR            m_Name[TMemory::HEAP_MAXNAME + 1];
};
}

inline static void* TMalloc(size_t size)
{
	return Toshi::TMemory::s_Context.s_cbMalloc(size);
}

inline static void* TCalloc(size_t nitems, size_t size)
{
	return Toshi::TMemory::s_Context.s_cbCalloc(nitems, size);
}

inline static void* TRealloc(void* mem, size_t newsize)
{
	return Toshi::TMemory::s_Context.s_cbRealloc(mem, newsize);
}

inline static void* TMemalign(size_t alignment, size_t size)
{
	return Toshi::TMemory::s_Context.s_cbMemalign(alignment, size);
}

inline static void TFree(void* mem)
{
	Toshi::TMemory::s_Context.s_cbFree(mem);
}

inline void* __CRTDECL operator new(size_t size, Toshi::TMemoryHeap* heap)
{
	return Toshi::TMemoryHeap::Malloc(heap, size);
}

inline void* __CRTDECL operator new[](size_t size, Toshi::TMemoryHeap* heap)
{
	return Toshi::TMemoryHeap::Malloc(heap, size);
}

inline void __CRTDECL operator delete(void* ptr, Toshi::TMemoryHeap* heap) noexcept
{
	heap->Free(ptr);
}

inline void __CRTDECL operator delete[](void* ptr, Toshi::TMemoryHeap* heap) noexcept
{
	heap->Free(ptr);

	TOSHI_NAMESPACE_END
