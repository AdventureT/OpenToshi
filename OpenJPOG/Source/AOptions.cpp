#include "pch.h"
#include "AOptions.h"
#include "Toshi/Plugins/PPropertyParser/PPropertyReader.h"

AOptions::Result AOptions::LoadOptions()
{
	LoadOptions(-1, -1, "Data", sm_szOptionsName);
	return Result();
}

AOptions::Result AOptions::LoadOptions(int a_iUnk, int a_iUnk2, const Toshi::TCString& a_szOptionsDir, const Toshi::TCString& a_szOptionsName)
{
	PPropertyReader reader = PPropertyReader();
	Toshi::TCString::Format("%s/%s.ini", a_szOptionsDir, a_szOptionsName);
	return Result();
}
