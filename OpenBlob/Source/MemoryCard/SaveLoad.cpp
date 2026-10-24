#include "pch.h"
#include "SaveLoad.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

SaveLoad* SaveLoad::s_instance;

// $deBlob: FUNCTION 005e3e60
// $deBlob: FUNCTION 005e3e70
// $deBlob: FUNCTION 005e3e80
// $deBlob: FUNCTION 005e3e90
// $deBlob: FUNCTION 005e3ea0
// $deBlob: FUNCTION 0053c960
// $deBlob: FUNCTION 008585c0
TDEFINE_CLASS_NORUNTIME( SaveLoad );

// $deBlob: FUNCTION 005e3f00
SaveLoad::SaveLoad()
{
	m_bEnabled = TTRUE;
	m_pCaller  = TNULL;
	m_iUnkC    = 0;

	TASSERT( s_instance == NULL );
	s_instance = this;
}

// $deBlob: FUNCTION 005e3eb0
// $deBlob: FUNCTION 005e3f60
SaveLoad::~SaveLoad()
{
	TASSERT( s_instance == this );
	s_instance = TNULL;
}
