#pragma once
#include "T2GUIElement.h"
#include "T2GUITextureSection.h"
#include "T2GUIRenderer.h"
#include "T2GUIContext.h"

#include <Toshi/TTask.h>

TOSHI_NAMESPACE_START

// de Blob's T2GUI: a 0xC bytes singleton opened by the game with a heap for materials. NOTE: Barnyard's T2GUI only has
// static CreateContext/DestroyContext and keeps the renderer in the context.
class T2GUI
{
public:
	T2GUI();
	~T2GUI();

	static T2GUI* Open( TMemoryHeap* a_pMemHeap );
	static void   Close();

	void           SetRenderer( T2GUIRenderer* a_pRenderer );
	T2GUIRenderer* GetRenderer() const;

	// $deBlob: FUNCTION 006f5080
	void Tick( TFLOAT a_fDeltaTime )
	{
		if ( m_pContext1 ) m_pContext1->GetRootElement()->Tick( a_fDeltaTime );
	}

	void Render();

	// Sets the context that is ticked and rendered, returns the previous one
	// $deBlob: FUNCTION 006f54c0
	T2GUIContext* SetContext( T2GUIContext* a_pContext )
	{
		T2GUIContext* pOld = m_pContext1;
		m_pContext1        = a_pContext;
		return pOld;
	}

	T2GUIContext* GetContext() const { return m_pContext1; }
	T2GUIElement* GetRootElement() const { return m_pContext1->GetRootElement(); }

	static T2GUIMaterial* CreateMaterial( TTexture* a_pTexture );
	static T2GUIMaterial* CreateMaterial( const TCHAR* a_szTextureName );
	static void           DestroyMaterial( TMaterial* a_pMaterial );
	static TTexture*      GetTexture( const TCHAR* a_szTextureName );
	static TUINT          GetWidth( T2GUIMaterial* a_pMaterial );
	static TUINT          GetHeight( T2GUIMaterial* a_pMaterial );

	static T2GUI* GetSingleton() { return s_Instance; }

private:
	static T2GUI*       s_Instance;
	static TMemoryHeap* s_pMemHeap;

private:
	T2GUIRenderer* m_pRenderer; // 0x0
	T2GUIContext*  m_pContext1; // 0x4
	T2GUIContext*  m_pContext2; // 0x8
};

TOSHI_NAMESPACE_END
