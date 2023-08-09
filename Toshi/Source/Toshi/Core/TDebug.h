#pragma once

namespace Toshi
{
	void TDebug_Printf(char const* text, ...) 
	{
		//TDebug_VPrintf(0, text, ...);
	}

	static void TDebug_VPrintf(uint32_t flag, const char*, char*)
	{

	}

	static void TDebug_VPrintfDirect(uint32_t flag, const char* format, char* args);

	class TDebug
	{
	public:

		inline static int s_iLogIndent = 0;
		inline static TFile* m_pDebugFile;
		inline static bool m_bEnableDebugFile;

		static void PrintIndent();
		static bool IsValidAddress(const void* address)
		{
			return address != TNULL && *(int*)address != 0xCDCDCDCD && address > (void*)0x50;
		}

		static bool AssertHandler(char* expression, char* file, int line, bool& ignored);
	};
}

