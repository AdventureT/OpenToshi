#pragma once
#include "Toshi/File/TFile.h"
#include "TCompress.h"

TOSHI_NAMESPACE_START

class TCompress_Decompress
{
public:
	static uintptr_t Decompress(TFile* file, TCompress::Header* header, TCHAR* buffer, TUINT32 bufferSize);
	static int8_t    GetHeader(TFile* file, TCompress::Header& btecHeader);
	static TINT      GetCommand(TFile* file, TBOOL& hasOffset, TUINT32& size, TINT& offset);
};

TOSHI_NAMESPACE_END
