#pragma once
#include "Toshi/T2ResourceManager.h"

TOSHI_NAMESPACE_START

class TModel;

//-----------------------------------------------------------------------------
// Purpose: a counted reference to a TModel resource. On PC TTexture isn't a T2Resource, so T2TexturePtr isn't
// used.
//-----------------------------------------------------------------------------
class T2ModelPtr : public T2ResourcePtr
{
public:
	T2ModelPtr( TINT a_iID = IDINVALID )
	    : T2ResourcePtr( a_iID )
	{}

	// $deBlob: FUNCTION 006b0830
	T2ModelPtr( const T2ModelPtr& a_rOther )
	    : T2ResourcePtr( a_rOther )
	{}

	T2ModelPtr& operator=( T2ModelPtr a_Other );

	// $deBlob: FUNCTION 006b0940
	void Release()
	{
		T2ResourceManager::GetSingleton()->DecRefCount( m_iResourceID );
		m_iResourceID = IDINVALID;
	}

	TBOOL IsValid() const { return m_iResourceID != IDINVALID; }

	TModel* GetData() const { return TSTATICCAST( TModel, T2ResourcePtr::GetData() ); }
	TModel* operator->() const { return GetData(); }
}; // 0x04

TOSHI_NAMESPACE_END
