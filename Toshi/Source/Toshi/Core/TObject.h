#pragma once
#include "TClass.h"

#include "Toshi/Core/Core.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Utils/TLog.h"

namespace Toshi
{
	class TObject
	{
	public:
		virtual TClass* GetClass() { return &s_Class; }
		virtual void Delete() { delete this; };
		virtual ~TObject() = default;

		inline static TObject* CreateTObject() { return nullptr; } 
		inline static TObject* CreateTObjectInPlace(void* ptr) { return nullptr; }

		inline bool IsExactly(TClass* toCompare) { return GetClass() == toCompare; }
	
	public:
		static constinit TClass s_Class;
	};

	template<class T, class Parent, STL::StringLiteral Name, uint32_t Version, bool Instantiable>
	class TGenericClassDerived : public Parent
	{
	public:
		virtual const char* GetName() const
		{
			return s_Class.GetName();
		}

		virtual uint16_t GetVersionMajor() const
		{
			return s_Class.GetVersionMajor();
		}

		virtual uint16_t GetVersionMinor() const
		{
			return s_Class.GetVersionMinor();
		}

		virtual TClass* GetClass()
		{
			return std::addressof(s_Class);
		}

		static TObject* CreateTObject()
		{
			if constexpr (Instantiable) { return new T(); }
			else { return nullptr; }
		}

		static TObject* CreateTObjectInPlace(void* ptr)
		{
			if constexpr (Instantiable) { return new (ptr) T(); }
			else { return nullptr; }
		}

		static TClass* GetClassStatic()
		{
			return std::addressof(s_Class);
		}

	public:
		static TClass s_Class;
	};

	template <class T, class Parent, STL::StringLiteral Name, uint32_t Version, bool Instantiable>
	TClass TGenericClassDerived<T, Parent, Name, Version, Instantiable>::s_Class = TClass(Name.value, &Parent::s_Class, Version, sizeof(T), T::CreateTObject, T::CreateTObjectInPlace, 0, 0);
}

