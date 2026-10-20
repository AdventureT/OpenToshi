#pragma once
#include "TModel.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: the list of every created TModel, linked through TModel::m_pNextModel/m_pPrevModel. Created by TRender's
// constructor. Not a TSingleton: the class has its own s_pSingleton.
//-----------------------------------------------------------------------------
class TModelManager
{
public:
	TModelManager();
	~TModelManager();

	void       AddModel( T2ModelPtr a_pModel );
	void       RemoveModel( T2ModelPtr a_pModel );
	T2ModelPtr GetNextModel( T2ModelPtr a_pModel );
	T2ModelPtr FindModel( const TCHAR* a_szName );

	const T2ModelPtr& GetFirstModel() const { return m_pHeadModel; }

	static TModelManager* GetSingleton() { return s_pSingleton; }

private:
	static TModelManager* s_pSingleton;

private:
	T2ModelPtr m_pHeadModel; // 0x00
}; // 0x04

TOSHI_NAMESPACE_END
