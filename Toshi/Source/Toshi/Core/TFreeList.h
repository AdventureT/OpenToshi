#pragma once
namespace Toshi
{
	class TFreeList
	{
	public:

		TFreeList(int size, int a_iInitialSize, int a_iGrowSize);

		void* Allocate(int a_iNumber, int size);

		int GetCapacity() const { return m_iCapacity; }
		int GetGrowSize() const { return m_iGrowSize; }
		int GetInitialSize() const { return m_iInitialSize; }

		void SetGrowSize(int a_iGrowSize) { a_iGrowSize < 0 ? m_iGrowSize = 8 : m_iGrowSize = a_iGrowSize; }
		void* New(uint32_t size);
		
		void Delete(void* a_pData);


	private:

		static inline TFreeList* ms_pRootFreeList = TNULL;

		TFreeList* m_pNextBlock;
		int m_iInitialSize;
		int m_iItemSize;
		int m_iCapacity;
		TFreeList* m_pLastNode;
		TFreeList* m_pRootBlock;
		int m_iGrowSize;
		TMemoryHeap* m_pMemoryHeap;
	};
}


