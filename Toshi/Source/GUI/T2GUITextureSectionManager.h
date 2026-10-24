#pragma once
#include "T2GUITextureSection.h"

TOSHI_NAMESPACE_START

class T2GUITextureSectionManager
{
public:
	struct TRBHeader
	{
		T2GUITextureSection* m_pSections;
		TUINT32              m_uiNumSections;
	};

	static TTRB*                g_trb;
	static TRBHeader*           g_defblock;
	static T2GUITextureSection* g_sections;

public:
	static TBOOL Open( const TCHAR* a_szFileName );
	static void  Close();

	static T2GUITextureSection* GetTextureSection( const TCHAR* a_szSectionName );

	static void CreateMaterials();
	static void DestroyMaterials();
	static void UpdateMaterials();
};

TOSHI_NAMESPACE_END
