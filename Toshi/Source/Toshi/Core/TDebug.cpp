#include "ToshiPCH.h"
#include "TDebug.h"
#include "TSystem.h"

void Toshi::TDebug::PrintIndent()
{
}

bool Toshi::TDebug::AssertHandler(char* expression, char* file, int line, bool& ignored)
{
    const char* ignore = ignored ? "(IGNORED)" : "";

    return false;
}

void Toshi::TDebug_VPrintfDirect(uint32_t flag, const char* format, char* args)
{
    if ((flag & 4) == 0)
    {
        if ((flag & 1) == 0)
        {
            if ((flag & 2) == 0)
            {
                TDebug::PrintIndent();
            }
            else
            {
                TDebug::s_iLogIndent--;
                TDebug::PrintIndent();
                if (!TDebug::m_pDebugFile || !TDebug::m_bEnableDebugFile)
                {
                    printf("%s", "} ");
                }
                else
                {
                    TDebug::m_pDebugFile->CPrintf("} ");
                }
            }
        }
        else
        {
            TDebug::PrintIndent();
            if (!TDebug::m_pDebugFile || !TDebug::m_bEnableDebugFile)
            {
                printf("%s", "{ ");
            }
            else
            {
                TDebug::m_pDebugFile->CPrintf("{ ");
            }
            TDebug::s_iLogIndent++;
        }
    }
    //vsprintf()
}
