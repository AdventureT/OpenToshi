#pragma once
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Purpose: a named console variable.
// The variables are statics of the files using them; AConsoleState keeps a list of them for the console.
//-----------------------------------------------------------------------------
class AConsoleVar
{
public:
	enum TYPE : TINT
	{
		TYPE_INT    = 0,
		TYPE_FLOAT  = 1,
		TYPE_BOOL   = 2,
		TYPE_STRING = 3,
	};

	static TCONSTEXPR_VAR TINT MAX_STRING = 0x20;

public:
	AConsoleVar( const TCHAR* a_szName, TFLOAT a_fValue );
	AConsoleVar( const TCHAR* a_szName, TINT a_iValue );
	AConsoleVar( const TCHAR* a_szName, TBOOL a_bValue );
	AConsoleVar( const TCHAR* a_szName, const TCHAR* a_szValue );

	// $deBlob: FUNCTION 005c2910
	void GetValue( TBOOL& a_rbValue ) const
	{
		TASSERT( TYPE_BOOL == m_eType );
		a_rbValue = m_bValue;
	}

	// NOTE: retail's destructor is empty (a shared stub); the static variables' atexit destructors call it
	~AConsoleVar();

	// $deBlob: FUNCTION 005b0bd0
	void SetFloat( TFLOAT a_fValue )
	{
		TASSERT( TYPE_FLOAT == m_eType );
		m_fValue = a_fValue;
	}

	// $deBlob: FUNCTION 005b0b80
	TINT SetInt( TINT a_iValue )
	{
		TASSERT( TYPE_INT == m_eType );
		m_iValue = a_iValue;
		return a_iValue;
	}

	// $deBlob: FUNCTION 005b0c20
	TBOOL SetBool( TBOOL a_bValue )
	{
		TASSERT( TYPE_BOOL == m_eType );
		m_bValue = a_bValue;
		return a_bValue;
	}

	void SetString( const TCHAR* a_szValue );

	// $deBlob: FUNCTION 0056c3e0
	TFLOAT GetFloat() const
	{
		TASSERT( TYPE_FLOAT == m_eType );
		return m_fValue;
	}

	// $deBlob: FUNCTION 005b08f0
	TINT GetInt() const
	{
		TASSERT( TYPE_INT == m_eType );
		return m_iValue;
	}

	// $deBlob: FUNCTION 005b0890
	TBOOL GetBool() const
	{
		TASSERT( TYPE_BOOL == m_eType );
		return m_bValue;
	}

	// $deBlob: FUNCTION 005b0980
	const TCHAR* GetString() const
	{
		TASSERT( TYPE_STRING == m_eType );
		return m_szValue;
	}

	// $deBlob: FUNCTION 005b09d0
	TYPE GetType() const { return m_eType; }
	// $deBlob: FUNCTION 0068fcb0
	const TCHAR* GetName() const { return m_szName; }

private:
	TYPE m_eType; // 0x00
	union
	{
		TINT   m_iValue;
		TFLOAT m_fValue;
		TBOOL  m_bValue;
		TCHAR  m_szValue[ MAX_STRING ];
	}; // 0x04
	const TCHAR* m_szName; // 0x24
};
