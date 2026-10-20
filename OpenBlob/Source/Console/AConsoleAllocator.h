#pragma once
#include <Toshi/T2Allocator.h>

//-----------------------------------------------------------------------------
// Purpose: the allocator of the console's strings and vectors: the CRT heap. The aligned
// versions fail (TNULL), the sizes are unknown (-1).
//-----------------------------------------------------------------------------
class AConsoleAllocator
    : public Toshi::T2Allocator
{
public:
	// $deBlob: FUNCTION 005afc50
	AConsoleAllocator() {}

	virtual void*  Malloc( size_t a_uiSize ) override;                          // +0x04
	virtual void*  Malloc( size_t a_uiSize, size_t a_uiAlignment ) override;    // +0x00
	virtual void   Free( void* a_pMem ) override;                               // +0x08
	virtual TBOOL  CanAllocate( size_t a_uiSize ) const override;               // +0x0C
	virtual void*  TryMalloc( size_t a_uiSize ) override;                       // +0x14
	virtual void*  TryMalloc( size_t a_uiSize, size_t a_uiAlignment ) override; // +0x10
	virtual size_t GetUsedSize() const override;                                // +0x18
	virtual size_t GetCapacity() const override;                                // +0x1C

	static AConsoleAllocator s_Instance;
};
