#pragma once

TOSHI_NAMESPACE_START

class TRefCounted
{
	TINT m_iRefCount;

public:
	TRefCounted() { m_iRefCount = 0; }
	TRefCounted(TRefCounted const&) { m_iRefCount = 0; }

	inline TINT         DecRefCount() { return m_iRefCount--; }
	inline TINT         GetRefCount() { return m_iRefCount; }
	inline TINT         IncRefCount() { return m_iRefCount++; }
	inline TRefCounted& operator=(TRefCounted const&) { return *this; }
};

TOSHI_NAMESPACE_END
