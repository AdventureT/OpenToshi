#include "ToshiPCH.h"
#include "TShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TShader::TShaderList TShader::sm_oShaderList;

// $deBlob: FUNCTION 006921a0
// $deBlob: FUNCTION 006921b0
// $deBlob: FUNCTION 006921c0
// $deBlob: FUNCTION 006921d0
// $deBlob: FUNCTION 006921e0
// $deBlob: FUNCTION 0053ebe0
// $deBlob: FUNCTION 00858ee0
TDEFINE_CLASS_NORUNTIME( TShader );

// $deBlob: FUNCTION 00692200
// $Barnyard: FUNCTION 006ccbb0
TShader::TShader()
{
	m_State = State_None;
	sm_oShaderList.AddShader( this );
}

// $deBlob: FUNCTION 00692290
// $deBlob: FUNCTION 006925a0
// $Barnyard: FUNCTION 006ccd90
TShader::~TShader()
{
	sm_oShaderList.RemoveShader( this );
}

// $deBlob: FUNCTION 006921f0
// $Barnyard: FUNCTION 006ccc30
void TShader::OnDestroy()
{
	m_State &= ~State_Created;
}

// $deBlob: FUNCTION 00692250
void TShader::Flush()
{
}

// $deBlob: FUNCTION 00692260
// $Barnyard: FUNCTION 006ccc20
TBOOL TShader::Create()
{
	m_State |= State_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 00692270
// $Barnyard: FUNCTION 006ccc40
TBOOL TShader::Validate()
{
	m_State |= State_Validated;
	return TTRUE;
}

// $deBlob: FUNCTION 00692280
// $Barnyard: FUNCTION 006ccc50
void TShader::Invalidate()
{
	m_State &= ~State_Validated;
}

void TShader::TShaderList::AddShader( TShader* pShader )
{
	TShader* pLastShader = m_pRoot;

	while ( pLastShader && pLastShader->m_pNextShader != TNULL )
		pLastShader = pLastShader->m_pNextShader;

	if ( pLastShader != TNULL )
		pLastShader->m_pNextShader = pShader;
	else
		m_pRoot = pShader;

	pShader->m_pNextShader = TNULL;
}

// $deBlob: FUNCTION 006920d0
// $Barnyard: FUNCTION 006ccb20
void TShader::TShaderList::RemoveShader( TShader* pShader )
{
	TShader* pCurrShader = m_pRoot;
	TShader* pPrevShader = TNULL;
	TBOOL    bFound      = TFALSE;

	while ( pCurrShader )
	{
		if ( pCurrShader == pShader )
		{
			bFound = TTRUE;
			break;
		}

		pPrevShader = pCurrShader;
		pCurrShader = pCurrShader->m_pNextShader;
	}

	if ( bFound )
	{
		if ( pPrevShader )
		{
			pPrevShader->m_pNextShader = pShader->m_pNextShader;
		}
		else
		{
			TASSERT( m_pRoot == pShader );
			m_pRoot = pShader->m_pNextShader;
		}

		pShader->m_pNextShader = TNULL;
	}
}

// Flushes all the shaders of the list
// $deBlob: FUNCTION 00692160
void TShader::TShaderList::FlushShaders()
{
	for ( TShader* pShader = m_pRoot; pShader != TNULL; pShader = pShader->GetNextShader() )
		pShader->Flush();
}

// $deBlob: FUNCTION 00692180
// $Barnyard: FUNCTION 006ccb90
void TShader::TShaderList::EnumerateShaders( t_EnumerateCallback a_fnCallback )
{
	for ( TShader* pShader = m_pRoot; pShader != TNULL; pShader = pShader->m_pNextShader )
		a_fnCallback( pShader );
}

TOSHI_NAMESPACE_END
