#pragma once
TOSHI_NAMESPACE_START

class ResourceName
{
	TCHAR  m_cNameLength; // 0x0
	TCHAR* m_sName;       // 0x1

public:
	const TCHAR* GetName() { return m_sName; }
	TINT         GetNameLength() { return m_cNameLength; }
};

TOSHI_NAMESPACE_END
