#pragma once
#include <Common/Base/hkBase.h>
#include <Common/Base/Memory/Memory/hkMemory.h>

TOSHI_NAMESPACE_START

// Havok's limited-memory listener set by TPhysicsManager::Initialise on its hkFreeListMemory
class TMemoryListener
    : public hkLimitedMemoryListener
{
public:
	HK_DECLARE_SYSTEM_ALLOCATOR();

	// $deBlob: FUNCTION 0072f7a0
	virtual ~TMemoryListener() = default;

	virtual void cannotAllocate( hkMemory* a_pMem, hk_size_t a_uiSize ) override;
	virtual void allocationFailure( hkMemory* a_pMem, hk_size_t a_uiSize ) override;
};

TOSHI_NAMESPACE_END
