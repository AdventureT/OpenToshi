#include "ToshiPCH.h"
#include "TShader.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00692200
TShader::TShader()
{
	m_State = State::None;
	sm_oShaderList.AddShader(this);
}

// $deBlob: FUNCTION 00692290
TShader::~TShader()
{
	sm_oShaderList.RemoveShader(this);
}

// $deBlob: FUNCTION 006921f0
void TShader::OnDestroy()
{
	m_State.Unset(State::Created);
}

// $deBlob: FUNCTION 00692250
void TShader::Flush()
{}

// $deBlob: FUNCTION 00692260
TBOOL TShader::Create()
{
	m_State.Set(State::Created);
	return TTRUE;
}

// $deBlob: FUNCTION 00692270
TBOOL TShader::Validate()
{
	m_State.Set(State::Validated);
	return TTRUE;
}

// $deBlob: FUNCTION 00692280
void TShader::Invalidate()
{
	m_State.Unset(State::Validated);
}

void TShader::TShaderList::AddShader(TShader* pShader)
{
	TShader* pLastShader = m_pRoot;

	while (pLastShader && pLastShader->m_pNextShader != TNULL) pLastShader = pLastShader->m_pNextShader;

	if (pLastShader != TNULL)
		pLastShader->m_pNextShader = pShader;
	else
		m_pRoot = pShader;

	pShader->m_pNextShader = TNULL;
}

void TShader::TShaderList::RemoveShader(TShader* pShader)
{
	TShader* pCurrShader = m_pRoot;
	TShader* pPrevShader = TNULL;
	TBOOL    bFound      = TFALSE;

	while (pCurrShader)
	{
		if (pCurrShader == pShader)
		{
			bFound = TTRUE;
			break;
		}

		pPrevShader = pCurrShader;
		pCurrShader = pCurrShader->m_pNextShader;
	}

	if (bFound)
	{
		if (pPrevShader)
		{
			TASSERT(m_pRoot != pShader);
			pPrevShader->m_pNextShader = pShader->m_pNextShader;
		}
		else
		{
			TASSERT(m_pRoot == pShader);
			m_pRoot = pShader->m_pNextShader;
		}
	}
}

TOSHI_NAMESPACE_END
