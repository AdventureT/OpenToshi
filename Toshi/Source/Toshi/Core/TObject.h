#pragma once
#include "TClass.h"

#include "Toshi/Core/Core.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Utils/TLog.h"

#define TOBJECT(class_name, parent, instantiable) class class_name : public Toshi::TGenericClassDerived<class_name, parent, #class_name, TMAKEVERSION(1, 0), instantiable>

TOSHI_NAMESPACE_START

class TALIGNAS(4) TObject
{
public:
	virtual TClass* GetClass() { return &s_Class; }
	virtual void    Delete() { delete this; };
	virtual ~TObject() = default;

	static TObject* CreateTObject() { return TNULL; }
	static TObject* CreateTObjectInPlace(void* ptr) { return TNULL; }

	TBOOL IsExactly(TClass* pClass) { return GetClass() == pClass; }
	TBOOL IsA(TClass* pClass) { return GetClass()->IsA(pClass); }

public:
	static constinit TClass s_Class;
};

template <class T, class Parent, STL::StringLiteral Name, TUINT32 Version, TBOOL Instantiable> class TGenericClassDerived : public Parent
{
public:
	virtual TClass* GetClass() { return std::addressof(s_Class); }

	static TObject* CreateTObject()
	{
		if constexpr (Instantiable)
		{
			void* pPtr = TMemalign(alignof(T), sizeof(T));
			return new (pPtr) T();
		}
		else
		{
			TASSERT(TFALSE, "This TObject cannot be created from TClass");
			return TNULL;
		}
	}

	static TObject* CreateTObjectInPlace(void* a_pPtr)
	{
		if constexpr (Instantiable)
		{
			return new (a_pPtr) T();
		}
		else
		{
			TASSERT(TFALSE, "This TObject cannot be created from TClass");
			return TNULL;
		}
	}

	static TClass* GetClassStatic() { return std::addressof(s_Class); }

public:
	static TClass s_Class;
};

template <class T, class Parent, STL::StringLiteral Name, TUINT32 Version, TBOOL Instantiable> TClass TGenericClassDerived<T, Parent, Name, Version, Instantiable>::s_Class =
    TClass(Name.value, &Parent::s_Class, Version, sizeof(T), T::CreateTObject, T::CreateTObjectInPlace, 0, 0, alignof(T));

TOSHI_NAMESPACE_END
