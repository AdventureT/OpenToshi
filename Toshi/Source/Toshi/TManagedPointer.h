#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a reference count (Toshi\TManagedPointer.h, 4 bytes, no virtual functions in de Blob)
//-----------------------------------------------------------------------------
class TRefCounted
{
public:
	TRefCounted()
	    : m_iRefCount( 0 )
	{}

	TINT IncRefCount() { return ++m_iRefCount; }

	// $deBlob: FUNCTION 0078d3e0
	TINT DecRefCount()
	{
		TASSERT( m_iRefCount >= 1 );
		return --m_iRefCount;
	}

	TINT GetRefCount() const { return m_iRefCount; }

protected:
	TINT m_iRefCount; // 0x00
};

TOSHI_NAMESPACE_END
