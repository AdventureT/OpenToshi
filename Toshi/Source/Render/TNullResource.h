#pragma once

#include "Render/TResource.h"

TOSHI_NAMESPACE_START

// Resource without any data, used to group the system resources (VFactories, Shaders, Storage...)
class TNullResource : public TResource
{
public:
	TDECLARE_CLASS( TNullResource, TResource );

public:
	// $deBlob: FUNCTION 006b5760
	TNullResource() = default;
	~TNullResource();
};

TOSHI_NAMESPACE_END
