#pragma once
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

class TOSHI_API T2Allocator
{
public:
	// NOTE: MSVC puts overloaded virtuals into the vtable in reverse declaration order. Retail has
	// Malloc( size, alignment ) at +0x00, Malloc( size ) at +0x04 (same for TryMalloc at +0x10/+0x14).
	virtual void*  Malloc( size_t size )                      = 0; // +0x04
	virtual void*  Malloc( size_t size, size_t alignment )    = 0; // +0x00
	virtual void   Free( void* ptr )                          = 0;
	virtual TBOOL  CanAllocate( size_t size ) const           = 0;
	virtual void*  TryMalloc( size_t size )                   = 0;
	virtual void*  TryMalloc( size_t size, size_t alignment ) = 0;
	virtual size_t GetUsedSize() const                        = 0;
	virtual size_t GetCapacity() const                        = 0;

	// NOTE: retail calls Malloc( sizeof( T ) ) without the alignment
	// $Barnyard: FUNCTION 006bc0d0
	// $Barnyard: FUNCTION 00554550
	// $deBlob: FUNCTION 005d9f90
	// $deBlob: FUNCTION 005dff50
	// $deBlob: FUNCTION 005dffe0
	// $deBlob: FUNCTION 006a9a00
	// $deBlob: FUNCTION 0075ee50
	// $deBlob: FUNCTION 007e3e60
	template <class T, class... Args>
	T* New( Args&&... args )
	{
		void* mem    = Malloc( sizeof( T ), TALIGNOF( T ) );
		T*    object = new ( mem ) T( std::forward<Args>( args )... );
		return object;
	}

	template <class T>
	void Delete( T* ptr )
	{
		ptr->~T();
		Free( ptr );
	}
};

class TOSHI_API T2GlobalAllocator : public T2Allocator
{
public:
	// NOTE: explicitly defaulted to carry the marker
	// $deBlob: FUNCTION 006f20c0
	T2GlobalAllocator() = default;

	virtual void*  Malloc( size_t size ) override;
	virtual void*  Malloc( size_t size, size_t alignment ) override;
	virtual void   Free( void* ptr ) override;
	virtual TBOOL  CanAllocate( size_t size ) const override;
	virtual void*  TryMalloc( size_t size ) override;
	virtual void*  TryMalloc( size_t size, size_t alignment ) override;
	virtual size_t GetUsedSize() const override;
	virtual size_t GetCapacity() const override;
};

extern T2GlobalAllocator ms_oGlobalAllocator;

TFORCEINLINE T2Allocator* GetGlobalAllocator()
{
	return &ms_oGlobalAllocator;
}

TOSHI_NAMESPACE_END
