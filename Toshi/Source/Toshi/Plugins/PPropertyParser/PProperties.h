#pragma once

#include "Toshi/Core/TFreeList.h"	

class PProperties : 
	public Toshi::TGenericClassDerived<PProperties, Toshi::TObject, "PProperties", TMAKEVERSION(1, 0), TTRUE>
{
public:

	/*static Toshi::TFreeList& GetFreeList() { return ms_oFreelist; }
private:
	static Toshi::TFreeList ms_oFreelist;*/
};

class PProperty
{

};

