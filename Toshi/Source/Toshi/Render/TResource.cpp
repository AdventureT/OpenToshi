#include "ToshiPCH.h"
#include "TResource.h"
#include "TRender.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006b57f0
TResource::~TResource()
{
	m_State |= TResourceState_Dead;
}

// $deBlob: FUNCTION 006b51d0
TBOOL TResource::Create()
{
	TASSERT(TFALSE == IsCreated(), "This resource is already created");

	m_State |= TResourceState_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 006b5210
TBOOL TResource::Validate()
{
	TASSERT(IsDying() == TFALSE, "Resource is dying and cannot be validated");
	TASSERT(Parent() == TNULL || Parent()->IsDying() == TFALSE, "Parent resource is dying");

	if (IsDying()) return TFALSE;

	m_State |= TResourceState_Valid;
	return TTRUE;
}

// $deBlob: FUNCTION 006b5820
void TResource::Invalidate()
{
	m_State &= ~TResourceState_Valid;
}

// $deBlob: FUNCTION 006b5320
void TResource::DestroyResource()
{
	GetRenderer()->DestroyResource(this);
}

// $deBlob: FUNCTION 006b5620
void TResource::OnDestroy()
{
	TASSERT(TTRUE == IsCreated(), "Tried to destroy resource that wasn't created");

	m_State &= ~TResourceState_Created;
	m_State |= TResourceState_Dying;
}

// $deBlob: FUNCTION 006b5350
void TResource::SetName(const TCHAR* name)
{
	// 006b5350
	if (name == TNULL)
	{
		name = "res:";

		TCHAR UIdStr[12];
		T2String8::IntToString(m_UId, UIdStr, 0xE, 10);
	}

	TASSERT(TStringManager::String8Length(name) <= MAXNAMELEN, "Name is too long");
	TStringManager::String8Copy(m_Name, name, -1);
}

TOSHI_NAMESPACE_END
