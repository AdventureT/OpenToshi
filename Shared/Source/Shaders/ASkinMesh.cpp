#include "pch.h"
#include "ASkinMesh.h"

// $deBlob: FUNCTION 00600f80
ASkinMesh::ASkinMesh()
{
	m_uiFlags        = 0;
	m_uiMaxVertices  = 0;
	m_uiNumSubMeshes = 0;
	m_pVertexPool    = TNULL;
}

// $deBlob: FUNCTION 00600e40
TBOOL ASkinMesh::Validate()
{
	if (!m_State.IsSet(State::Validated))
		m_State.Set(State::Validated);

	return TTRUE;
}

// $deBlob: FUNCTION 00600e60
void ASkinMesh::Invalidate()
{
	if (m_State.IsSet(State::Validated))
		m_State.Unset(State::Validated);
}
