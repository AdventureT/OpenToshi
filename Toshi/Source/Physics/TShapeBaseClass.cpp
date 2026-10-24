#include "ToshiPCH.h"
#include "TShapeBaseClass.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00733ca0
// $deBlob: FUNCTION 00733210
TShapeBaseClass::~TShapeBaseClass()
{
}

// $deBlob: FUNCTION 00733220
void TShapeBaseClass::AddRef( TShapeDataCache* a_pCache )
{
	if ( !m_pCache )
	{
		m_pCache = a_pCache;
		a_pCache->PushBack( this );
	}

	m_iRefCount++;
}

// $deBlob: FUNCTION 00733280
void TShapeBaseClass::Release()
{
	if ( m_iRefCount-- == 1 )
	{
		if ( m_pCache )
			m_pCache->FindAndErase( this );

		delete this;
	}
}

TOSHI_NAMESPACE_END
