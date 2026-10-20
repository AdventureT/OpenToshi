#include "pch.h"
#include "AJob.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005e1540
AJob::AJob()
{
	m_uiFlags = 0;
}

// $deBlob: FUNCTION 005e1560
// $deBlob: FUNCTION 005e15f0
AJob::~AJob()
{
}

// $deBlob: FUNCTION 005e1660
void AJob::SetReady()
{
	m_uiFlags |= FLAG_Ready;
}

// $deBlob: FUNCTION 005e1650
void AJob::SetDone()
{
	m_uiFlags |= FLAG_Done;
}
