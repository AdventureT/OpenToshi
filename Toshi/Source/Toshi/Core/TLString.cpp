#include "ToshiPCH.h"
#include "TLString.h"
#include <Toshi/File/TFileLexerUTF8.h>

bool Toshi::TLocale::LoadInfo(const TCString& a_rFileName)
{
    TFile* pFile = TFile::Create(a_rFileName);
    TASSERT(pFile != TNULL);
    TOSHI_INFO("TLoacle::LoadInfo()...");
    TFileLexerUTF8 lexer = TFileLexerUTF8(pFile, 2);
    return false;
}
