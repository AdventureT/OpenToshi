#pragma once

#include "Toshi/Core/TFreeList.h"
#include "Toshi/Core/TQList.h"
#include <Toshi/Plugins/PPropertyParser/PPropertyName.h>

class PPropertyValue;

class PProperties : 
	public Toshi::TGenericClassDerived<PProperties, Toshi::TObject, "PProperties", TMAKEVERSION(1, 0), TTRUE>
{
public:

	class PProperty : public Toshi::TQList<PProperty>::TNode
	{
	public:

		PProperty()
		{

		}

		static void* operator new(uint32_t size, char*, int)
		{
			return ms_oFreelist.New(size);
		}

		static void* operator new(uint32_t size)
		{
			return ms_oFreelist.New(size);
		}

		static void* operator new(uint32_t size, void* data)
		{
			// No fuck you
			return data;
		}

		static void operator delete(void* data)
		{
			ms_oFreelist.Delete(data);
		}

		static Toshi::TFreeList& GetFreeList() { return ms_oFreelist; }

		static inline Toshi::TFreeList ms_oFreelist;
	};

	PProperties()
	{
		
	}

	static void* operator new(uint32_t size, char*, int)
	{
		return ms_oFreelist.New(size);
	}

	static void* operator new(uint32_t size)
	{
		return ms_oFreelist.New(size);
	}

	static void* operator new(uint32_t size, void* data)
	{
		// No fuck you
		return data;
	}

	static void operator delete(void* data)
	{
		ms_oFreelist.Delete(data);
	}

	static Toshi::TFreeList& GetFreeList() { return ms_oFreelist; }

	const PPropertyValue* GetProperty(const Toshi::TPCString& a_szPropertyName);
	const PPropertyValue* GetProperty(const PPropertyName& a_szPropertyName) const;

private:
	static inline Toshi::TFreeList ms_oFreelist;

	PProperties* m_pParentProps;                    // 0x8
	Toshi::TQList<PProperties::PProperty> m_oProps; // 0xC
	int m_iPropCount;                               // 0x14
};


