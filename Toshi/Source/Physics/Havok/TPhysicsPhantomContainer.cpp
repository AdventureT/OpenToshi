#include "ToshiPCH.h"
#include "TPhysicsPhantomContainer.h"
#include "TPhysicsManager.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 007355d0
TPhysicsPhantomContainer* TPhysicsPhantomContainer::Create()
{
	return new ( TMalloc( sizeof( TPhysicsPhantomContainer ), TPhysicsManager::GetHeap() ) ) TPhysicsPhantomContainer();
}

// $deBlob: FUNCTION 00735580
void TPhysicsPhantomContainer::Destroy( TPhysicsPhantomContainer* a_pContainer )
{
	if ( a_pContainer )
		delete a_pContainer;
}

TOSHI_NAMESPACE_END
