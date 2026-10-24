#pragma once
#include <Toshi/T2DList.h>
#include <Toshi/TString8.h>
#include <Toshi/T2ResourceHelper.h>
#include <Render/TModel.h>

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: a model file shared by its instances. Owned by AModelFactory; loads the TModel of the same name from the
// TModelManager and creates the TModelInstance of each AModelInstance2.
//-----------------------------------------------------------------------------
class AModel2
    : public Toshi::T2DList<AModel2>::Node
{
public:
	static TCONSTEXPR_VAR TUINT8 FLAG_Loaded = 1;
	static TCONSTEXPR_VAR TUINT8 FLAG_Render = 2;

	friend class AModelFactory;
	friend class AModelInstance2;

public:
	AModel2();
	virtual ~AModel2();

	void             Load();
	void             Unload();
	AModelInstance2* CreateInstance( TBOOL a_bCreateSoundBreakpoints, TUINT a_eType, TBOOL a_bCreateAnimInterface );
	void             DestroyInstance( AModelInstance2* a_pInstance );
	void             RenderAllInstances();

	// Renders the instances with FLAG_RenderOnLoadingScreen (the loading screen)
	void RenderLoadingScreenInstances();

	// Moves a_pInstance to the front (renderable, like CreateInstance) or the back of m_lInstances
	void SortInstance( AModelInstance2* a_pInstance, TBOOL a_bRenderable );

	TBOOL HasInstances() const { return !m_lInstances.IsEmpty(); }
	TBOOL IsRendered() const { return HASANYFLAG( m_uiFlags, FLAG_Render ); }
	// $deBlob: FUNCTION 0056ea10
	TBOOL IsLoaded() const { return HASANYFLAG( m_uiFlags, FLAG_Loaded ); }

	const Toshi::TString8& GetFileName() const { return m_FileName; }
	Toshi::TModel*         GetModel() const { return m_pModel.GetData(); }

	// The model if it's loaded, else an empty pointer
	// $deBlob: FUNCTION 0056e940
	Toshi::T2ModelPtr GetModelPtr() const { return IsLoaded() ? m_pModel : Toshi::T2ModelPtr(); }

private:
	void Initialise( Toshi::T2ModelPtr a_pModel );

	static Toshi::TModelInstance* CreateModelInstance( Toshi::TModel* a_pModel );

private:
	Toshi::T2ModelPtr               m_pModel;     // 0x0C
	Toshi::TString8                 m_FileName;   // 0x10
	Toshi::T2DList<AModelInstance2> m_lInstances; // 0x1C
	TUINT8                          m_uiFlags;    // 0x24
};                                                // 0x28
