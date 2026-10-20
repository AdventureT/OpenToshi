#pragma once
#include "Animation/AModel2.h"
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: creates the model instances by file name, sharing one AModel2 per file. Created by AAppInitState, destroyed
// by AApplication.
//-----------------------------------------------------------------------------
class AModelFactory
    : public Toshi::TSingleton<AModelFactory>
{
public:
	AModelFactory();
	~AModelFactory();

	AModelInstance2* CreateInstance( const TCHAR* a_szFileName, TBOOL a_bCreateSoundBreakpoints, TBOOL a_bCreateAnimInterface );
	// Like CreateInstance, but a new model goes to m_UnmanagedModels, and a_eType picks the instance class
	AModelInstance2* CreateUnmanagedInstance( const TCHAR* a_szFileName, TBOOL a_bCreateSoundBreakpoints, TUINT a_eType, TBOOL a_bCreateAnimInterface );
	void             DestroyInstance( AModelInstance2* a_pInstance );


	// CreateInstance with a_szFileName as the model's name as it is (no directory stripped); a_uiUnused isn't read
	AModelInstance2* CreateInstanceByName( TUINT a_uiUnused, const TCHAR* a_szFileName, TBOOL a_bCreateSoundBreakpoints, TBOOL a_bCreateAnimInterface );

	// Whether a model of the file is loaded (m_Models / m_UnmanagedModels) and which one
	TBOOL    HasModel( const TCHAR* a_szFileName );
	TBOOL    HasUnmanagedModel( const TCHAR* a_szFileName );
	AModel2* FindModel( const TCHAR* a_szFileName );
	AModel2* FindUnmanagedModel( const TCHAR* a_szFileName );
	void     Render();

	// Renders the instances with AModelInstance2::FLAG_RenderOnLoadingScreen of every model (the loading screen)
	void RenderLoadingScreenInstances();

	// Returns the file name without its directory
	static Toshi::TString8 GetFilename( const TCHAR* a_szPath );

private:
	Toshi::T2DList<AModel2> m_Models;          // 0x00
	Toshi::T2DList<AModel2> m_UnmanagedModels; // 0x08
};                                             // 0x10
