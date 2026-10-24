#include "ToshiPCH.h"
#include "TMesh.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006924b0
// $deBlob: FUNCTION 006924c0
// $deBlob: FUNCTION 006924d0
// $deBlob: FUNCTION 006924e0
// $deBlob: FUNCTION 006924f0
// $deBlob: FUNCTION 0053eba0
// $deBlob: FUNCTION 00858ed0
TDEFINE_CLASS_NORUNTIME( TMesh );

// $deBlob: FUNCTION 005fb9d0
// $Barnyard: FUNCTION 005f5e80
// $Barnyard: FUNCTION 005f0340
TMesh::~TMesh()
{
	// NOTE: Barnyard clears m_pOwnerShader, m_pMaterial and m_State here; de Blob's destructor is empty
}

// $deBlob: FUNCTION 0055f4d0
// $Barnyard: FUNCTION 005effd0
TBOOL TMesh::Validate()
{
	m_State |= State_Validated;
	return TTRUE;
}

// $deBlob: FUNCTION 0055f420
void TMesh::Invalidate()
{
	m_State &= ~State_Validated;
}

// $deBlob: FUNCTION 0055f3b0
// $Barnyard: FUNCTION 005efff0
TBOOL TMesh::Create()
{
	m_State |= State_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 005696d0
TBOOL TMesh::Render()
{
	return TTRUE;
}

// $deBlob: FUNCTION 0055f450
// $Barnyard: FUNCTION 005f0000
void TMesh::OnDestroy()
{
	m_State &= ~State_Created;
}

// $deBlob: FUNCTION 00692500
// $Barnyard: FUNCTION 006ccd40
void TMesh::DestroyResource()
{
	Invalidate();
	OnDestroy();
	delete this;
}

void TMesh::SetMaterial( TMaterial* pMaterial )
{
	m_pMaterial = pMaterial;
}

TMaterial* TMesh::GetMaterial() const
{
	return m_pMaterial;
}

// $deBlob: FUNCTION 005678d0
// $deBlob: FUNCTION 00705e80
void TMesh::SetOwnerShader( TShader* pShader )
{
	TASSERT( TNULL == m_pOwnerShader );
	m_pOwnerShader = pShader;
}

// $deBlob: FUNCTION 005696b0
TBOOL TMesh::IsCreated() const
{
	return m_State & State_Created;
}

TBOOL TMesh::IsWater() const
{
	return m_State & State_IsWater;
}

TShader* TMesh::GetOwnerShader()
{
	return m_pOwnerShader;
}

TOSHI_NAMESPACE_END
