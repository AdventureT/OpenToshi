#pragma once
#include "TFile.h"
#include "Toshi/File/TCompress.h"

TOSHI_NAMESPACE_START

class TCompress_Compress
{
public:
	static constexpr TINT maxlength = 0x4000;
	static TINT           usemaxoffset;

public:
	static size_t Compress(TFile* file, TCHAR* data, TUINT32 size, TUINT32 unused, TBOOL isBigEndian);

private:
	static TINT WriteOffset(TUINT32 length, TINT offset, TCHAR*& data, TFile* file);
	static TINT Write(TUINT32 length, TCHAR*& data, TFile* file);
};

TOSHI_NAMESPACE_END
