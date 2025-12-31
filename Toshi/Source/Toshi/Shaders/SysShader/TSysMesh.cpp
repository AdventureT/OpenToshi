#include "ToshiPCH.h"
#include "TSysMesh.h"
#include "Toshi/Render/TRender.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0070ad80
TSysMesh::TSysMesh()
{
	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
	m_uiMaxIndices  = 0;
	m_pVertexPool   = TNULL;
	m_pIndexPool    = TNULL;
}

// $deBlob: FUNCTION 0070aab0
TBOOL TSysMesh::Validate()
{
	if (!m_State.IsSet(State::Validated)) m_State.Set(State::Validated);

	return TTRUE;
}

// $deBlob: FUNCTION 0070aad0
void TSysMesh::Invalidate()
{
	if (m_pVertexPool != TNULL && m_pIndexPool != TNULL) Deallocate();

	if (m_State.IsSet(State::Validated)) m_State.Unset(State::Validated);
}

// $deBlob: FUNCTION 0070ab00
TBOOL TSysMesh::Create(FLAGS uiFlags, TUINT16 uiMaxVertices, TUINT16 uiMaxIndices)
{
	TASSERT(TFALSE == IsCreated());

	m_uiFlags       = uiFlags;
	m_uiMaxVertices = uiMaxVertices;
	m_uiMaxIndices  = uiMaxIndices;

	if (Allocate())
	{
		return TTRUE;
	}
	else
	{
		return TFALSE;
	}
}

// $deBlob: FUNCTION 0070ab70
TBOOL TSysMesh::Lock(TLockBuffer* pLockBuffer)
{
	TASSERT(!(m_uiFlags & FLAGS_LOCKED));
	TIMPLEMENT();

	return TTRUE;
}

// $deBlob: FUNCTION 0070abf0
void TSysMesh::Unlock(TUINT16 uiNumVertices, TUINT16 uiNumIndices)
{
	TASSERT(m_uiFlags & FLAGS_LOCKED);
	TASSERT(uiNumVertices >= 3);
	TASSERT(uiNumIndices >= 3);

	TIMPLEMENT();
}

// $deBlob: FUNCTION 0070ad50
TResource* TSysMesh::GetVertexPool() const
{
	return m_pVertexPool;
}

// $deBlob: FUNCTION 0070ad60
TResource* TSysMesh::GetIndexPool() const
{
	return m_pIndexPool;
}

// $deBlob: FUNCTION 0070add0
TBOOL TSysMesh::Allocate()
{
	TASSERT(0 != m_uiMaxVertices);
	TASSERT(3 <= m_uiMaxIndices);
	TASSERT(TNULL == m_pVertexPool);
	TASSERT(TNULL == m_pIndexPool);

	auto pRender   = TRender::GetSingleton();
	auto pVertices = pRender->GetSystemResource(TRender::SYSRESOURCE_VFSYSVNDUV1);

	TASSERT(TFALSE, "Not used in De Blob on Windows");
	TIMPLEMENT();

	return TTRUE;
}

// $deBlob: FUNCTION 0070af50
void TSysMesh::Deallocate()
{
	if (m_pVertexPool != TNULL)
	{
		TRender::GetSingleton()->DestroyResource(m_pVertexPool);
		m_pVertexPool = TNULL;
	}

	if (m_pIndexPool != TNULL)
	{
		TRender::GetSingleton()->DestroyResource(m_pIndexPool);
		m_pIndexPool = TNULL;
	}

	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
	m_uiMaxIndices  = 0;
}

TOSHI_NAMESPACE_END
