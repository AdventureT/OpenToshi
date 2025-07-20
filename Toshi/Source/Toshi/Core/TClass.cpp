#include "ToshiPCH.h"
#include "TClass.h"
#include "TSystem.h"
#include "Toshi/Typedefs.h"

TOSHI_NAMESPACE_START

TClass::TClass(const TCHAR* a_szName, TClass* a_pParent, TUINT32 a_uiVersion, TUINT32 a_uiSize, t_CreateTObject a_fCreate, t_CreateTObjectInPlace a_fCreateInPlace, t_InitializeStatic a_fInit, t_UninitializeStatic a_fUninit, TUINT32 a_uiAlignment)
{
	m_Name          = a_szName;
	m_Create        = a_fCreate;
	m_CreateInPlace = a_fCreateInPlace;
	m_Initialize    = a_fInit;
	m_Uninitialize  = a_fUninit;
	m_Parent        = a_pParent;
	m_Version       = a_uiVersion;
	m_Size          = a_uiSize;
	m_Initialized   = TFALSE;
	m_Alignment     = a_uiAlignment;

	if (m_Parent)
	{
		// check if it's not attached yet
		TClass* tClass = m_Parent->m_LastAttached;
		while (tClass != TNULL)
		{
			if (tClass == this) return;
			tClass = tClass->m_Previous;
		}

		m_Previous               = m_Parent->m_LastAttached;
		m_Parent->m_LastAttached = this;
	}
}

TClass::~TClass()
{
	m_LastAttached = TNULL;
	m_Previous     = TNULL;
	m_Parent       = TNULL;

	// Uninitialize static
	if (m_Initialized && m_Uninitialize)
	{
		m_Initialized = TFALSE;
		m_Uninitialize();
	}
}

void TClass::Initialize()
{
	if (m_Initialize)
	{
		TASSERT(TFALSE == m_Initialized);
		m_Initialize();
		m_Initialized = TTRUE;
	}
}

void TClass::RecurseTree(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom)
{
	TBOOL valid = fCheck(this, custom);

	if (valid)
	{
		if (fBaseBegin) fBaseBegin(this, custom);
		RecurseTree2(fCheck, fBaseBegin, fBaseEnd, custom);
		if (fBaseEnd) fBaseEnd(this, custom);
	}
}

void TClass::RecurseTree2(t_RecurceTreeCheck fCheck, t_RecurceTreeBaseBeginCb fBaseBegin, t_RecurceTreeBaseEndCb fBaseEnd, void* custom)
{
	TClass* tClass = m_LastAttached;

	while (tClass != TNULL)
	{
		if (fCheck) fCheck(tClass, custom);

		if (tClass->m_LastAttached)
		{
			if (fBaseBegin) fBaseBegin(tClass, custom);
			tClass->RecurseTree2(fCheck, fBaseBegin, fBaseEnd, custom);
			if (fBaseEnd) fBaseEnd(tClass, custom);
		}

		tClass = tClass->m_Previous;
	}
}

const TClass* TClass::FindRecurse(const TCHAR* const& name, const TClass* parent, TBOOL hasPrevious)
{
	while (parent)
	{
		TClass* previous   = hasPrevious ? parent->m_Previous : TNULL;
		TINT    difference = Toshi::TStringManager::String8CompareNoCase(parent->m_Name, name, -1);

		if (difference == 0)
		{
			return parent;
		}

		if (parent->m_LastAttached)
		{
			const TClass* result = FindRecurse(name, parent->m_LastAttached, TTRUE);

			if (result)
			{
				return result;
			}
		}

		parent = previous;
	}

	return TNULL;
}

const TClass* TClass::Find(const TCHAR* a_pcClassName, const TClass* parent)
{
	TASSERT(a_pcClassName[1] != 0);
	if (parent == TNULL) parent = &TObject::s_Class;
	return FindRecurse(a_pcClassName, parent, TFALSE);
}

TObject* TClass::CreateObject()
{
	if (m_Create != TNULL)
	{
		return m_Create();
	}

	return TNULL;
}

TBOOL TClass::IsA(const TClass* cmpClass) const
{
	if (cmpClass->m_LastAttached == TNULL)
	{
		return (this == cmpClass);
	}
	else
	{
		const TClass* pClass = this;

		while (pClass != TNULL)
		{
			if (pClass == cmpClass)
			{
				return TTRUE;
			}

			pClass = pClass->m_Parent;
		}
	}

	return TFALSE;
}

TUINT32 TClass::GetMaxSizeOfDerivedClasses()
{
	auto GetMaxSizeOfClass = [](TClass* a_pClass, void* a_pSize) -> TBOOL {
		if (*(TUINT32*)a_pSize < a_pClass->m_Size)
		{
			*(TUINT32*)a_pSize = a_pClass->m_Size;
		}

		return TTRUE;
	};

	TUINT32 uiClassSize = 0;
	if (GetMaxSizeOfClass(this, &uiClassSize))
	{
		RecurseTree2(GetMaxSizeOfClass, TNULL, TNULL, &uiClassSize);
	}

	return uiClassSize;
}

TUINT32 TClass::GetMaxAlignmentOfDerivedClasses()
{
	auto GetMaxAlignmentOfClass = [](TClass* a_pClass, void* a_pAlignment) -> TBOOL {
		if (*(TUINT32*)a_pAlignment < a_pClass->m_Alignment)
		{
			*(TUINT32*)a_pAlignment = a_pClass->m_Alignment;
		}

		return TTRUE;
	};

	TUINT32 uiAlignment = 0;
	if (GetMaxAlignmentOfClass(this, &uiAlignment))
	{
		RecurseTree2(GetMaxAlignmentOfClass, TNULL, TNULL, &uiAlignment);
	}

	return uiAlignment;
}

TBOOL TClass::TryInitialize(TClass* tClass)
{
	if (!tClass->IsInitialized())
	{
		tClass->Initialize();
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
