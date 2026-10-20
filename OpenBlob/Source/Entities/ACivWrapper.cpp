#include "pch.h"
#include "ACivWrapper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ACivWrapper ) == 0x70 );

// $deBlob: FUNCTION 00763780
ACivWrapper::ACivWrapper()
    : m_RespawnData()
    , m_uiColour( 0 )
    , m_fRadius( 0.0f )
    , m_pPooledEntity( TNULL )
    , m_iParam( 0 )
{
}

// $deBlob: FUNCTION 007639a0
ACivWrapper::~ACivWrapper()
{
}

// $deBlob: FUNCTION 00763af0
const Toshi::TVector4* ACivWrapper::GetPosition()
{
	return &m_vPosition;
}

// $deBlob: FUNCTION 00763b10
TFLOAT ACivWrapper::GetRadius()
{
	return m_fRadius;
}
