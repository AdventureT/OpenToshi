#pragma once

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a list of formatted error messages (one buffer for the strings, one for their offsets)
//-----------------------------------------------------------------------------
class TError : public TSingleton<TError>
{
public:
	TError( TINT a_iStringBufferSize, TINT a_iMaxErrors );
	~TError();

	const TCHAR* GetError( TINT a_iIndex );
	void         AddError( const TCHAR* a_szFormat, ... );
	// $deBlob: FUNCTION 00663430
	void SetErrorOffsets( TINT* a_pErrorOffsets ) { m_pErrorOffsets = a_pErrorOffsets; }

private:
	TCHAR* m_pStringBuffer;     // 0x00
	TINT   m_iStringBufferSize; // 0x04
	TINT*  m_pErrorOffsets;     // 0x08
	TINT   m_iMaxErrors;        // 0x0C
	TINT   m_iStringBufferUsed; // 0x10
	TINT   m_iNumErrors;        // 0x14
};                              // 0x18

TOSHI_NAMESPACE_END
