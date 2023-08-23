#pragma once

#include "Toshi/Core/TFreeList.h"	

class PProperties : 
	public Toshi::TGenericClassDerived<PProperties, Toshi::TObject, "PProperties", TMAKEVERSION(1, 0), TTRUE>
{
public:

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
private:
	static inline Toshi::TFreeList ms_oFreelist;

	PProperties* m_pParentProps; // 0x8
	int m_iPropCount;            // 0x14
};

class PProperty
{

};

