#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a sorted table of lower case strings, looked up by binary search
//-----------------------------------------------------------------------------
class TStringTable
{
public:
	// $deBlob: FUNCTION 00828b80
	TStringTable( const TCHAR* const* a_ppStrings, TINT a_iNumStrings )
	{
		m_pStrings    = a_ppStrings;
		m_iNumStrings = a_iNumStrings;
	}

	// Returns the index of a_pString or -1
	TINT GetStringID( const TCHAR* a_pString );

	const TCHAR* GetStringFromID( TINT a_ID );

	// Asserts that the strings are lower case and sorted
	void CheckIfDataValid();

private:
	const TCHAR* const* m_pStrings;    // 0x0
	TINT                m_iNumStrings; // 0x4
};

TOSHI_NAMESPACE_END
