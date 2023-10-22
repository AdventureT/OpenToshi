#pragma once

namespace Toshi {

	class TMemoryContext
	{
	private:
		inline static auto MallocNative = [](size_t size) -> void* { return malloc(size); };
		inline static auto CallocNative = [](size_t nitems, size_t size) -> void* { return calloc(nitems, size); };
		inline static auto ReallocNative = [](void* ptr, size_t size) -> void* { return realloc(ptr, size); };
		inline static auto MemalignNative = [](size_t alignment, size_t size) -> void* { return malloc(size); };
		inline static auto FreeNative = [](void* ptr) -> void { free(ptr); };
		inline static auto IdkNative = [](void* ptr, size_t size) -> void {};

		friend class TMemory;

	public:
		typedef void* (*t_Malloc)(size_t size);
		typedef void* (*t_Calloc)(size_t nitems, size_t size);
		typedef void* (*t_Realloc)(void* ptr, size_t size);
		typedef void  (*t_Idk)(void* ptr, size_t size);
		typedef void* (*t_Memalign)(size_t alignment, size_t size);
		typedef void  (*t_Free)(void* ptr);

		t_Malloc s_cbMalloc = MallocNative;
		t_Calloc s_cbCalloc = CallocNative;
		t_Realloc s_cbRealloc = ReallocNative;
		t_Idk s_cbIdk = IdkNative;
		t_Memalign s_cbMemalign = MemalignNative;
		t_Free s_cbFree = FreeNative;

		void* s_Sysheap = TNULL;
		void* s_Heap = TNULL;
	};

}