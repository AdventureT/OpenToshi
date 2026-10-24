#pragma once

#define TDECLARE_FREELIST_ALLOCATOR( CLASS_NAME )                                                    \
public:                                                                                              \
	TFORCEINLINE void* operator new( size_t s ) { return ms_oFreeList.New( sizeof( CLASS_NAME ) ); } \
	TFORCEINLINE void* operator new( size_t s, void* where ) { return where; }                       \
                                                                                                     \
	TFORCEINLINE void operator delete( void* ptr ) { ms_oFreeList.Delete( ptr ); }                   \
	TFORCEINLINE void operator delete( void* ptr, void* where ) {}                                   \
                                                                                                     \
	static void SetupFreeList( TINT a_iCapacity, TINT a_iGrowSize )                                  \
	{                                                                                                \
		ms_oFreeList.SetCapacity( a_iCapacity );                                                     \
		ms_oFreeList.SetGrowSize( a_iGrowSize );                                                     \
	}                                                                                                \
                                                                                                     \
	static Toshi::TFreeList ms_oFreeList;

#define TDEFINE_FREELIST_ALLOCATOR( CLASS_NAME ) \
	Toshi::TFreeList CLASS_NAME::ms_oFreeList = Toshi::TFreeList( sizeof( CLASS_NAME ), 0, 8, #CLASS_NAME );

#define TDEFINE_FREELIST_ALLOCATOR1( CLASS_NAME, GROW_SIZE ) \
	Toshi::TFreeList CLASS_NAME::ms_oFreeList = Toshi::TFreeList( sizeof( CLASS_NAME ), 0, GROW_SIZE, #CLASS_NAME );

TOSHI_NAMESPACE_START

class TFreeList
{
public:
	struct Node
	{
		Node* pNext = TNULL;
	};

public:
	// NOTE: unknown object m_pHeapHolder points to, only its heap (+0x04) is used
	struct HeapHolder
	{
		void*        pUnknown; // 0x00
		TMemoryHeap* pHeap;    // 0x04
	};

public:
	TFreeList( TUINT a_uiItemSize, TINT a_iInitialSize, TINT a_iGrowSize, const TCHAR* a_pchName );
	~TFreeList();

	Node* Allocate( TINT a_iNumber, TINT a_iSize );
	void  SetCapacity( TINT a_iNewCapacity );

	// $deBlob: FUNCTION 006f2e10
	TINT GetCapacity() const { return m_iCapacity; }
	TINT GetGrowSize() const { return m_iGrowSize; }

	// $deBlob: FUNCTION 006f2e20
	void  SetGrowSize( TINT a_iGrowSize ) { a_iGrowSize < 0 ? m_iGrowSize = 8 : m_iGrowSize = a_iGrowSize; }
	void* New( TUINT a_uiSize );

	void Delete( void* a_Ptr );

	// Frees the memory chunks of this free list
	void FreeChunks();

	// Frees the memory chunks of every free list
	static void DeinitialiseAll();

	// Goes through every free list. The loop body (most likely logging) is compiled out of retail
	// $deBlob: FUNCTION 006f2c30
	// $Barnyard: FUNCTION 006c19c0
	static void DumpAll()
	{
		for ( TFreeList* pList = ms_pLastList; pList != TNULL; pList = pList->m_pPrevList )
			;
	}

private:
	static TFreeList* ms_pLastList;

#ifdef DEBLOB_COMMUNITY_PATCH
	// Set by DeinitialiseAll, Delete does nothing after that
	static TBOOL ms_bDeinitialised;
#endif // DEBLOB_COMMUNITY_PATCH

private:
	TFreeList*  m_pPrevList;   // 0x00
	TINT        m_iCapacity;   // 0x04
	TUINT       m_uiItemSize;  // 0x08
	Node*       m_pFreeNode;   // 0x0C
	Node*       m_pRootBlock;  // 0x10
	TINT        m_iGrowSize;   // 0x14
	HeapHolder* m_pHeapHolder; // 0x18
};

TOSHI_NAMESPACE_END
