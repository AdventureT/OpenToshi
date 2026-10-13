#pragma once
#include "T2GUIElement.h"
#include "T2GUIRenderer.h"
#include "T2GUIContext.h"

TOSHI_NAMESPACE_START

class T2GUI : public TSingleton<T2GUI>
{
public:
	T2GUI();
	~T2GUI();

	// $deBlob: FUNCTION 006f4fb0
	static T2GUI* Open(TMemoryHeap* pMemoryHeap)
	{
		s_pMemHeap = pMemoryHeap;
		return T2GUI::CreateSingleton();
	}

	// $deBlob: FUNCTION 006f5020
	static void Close()
	{
		T2GUI::DestroySingleton();
		s_pMemHeap = TNULL;
	}

	// $deBlob: FUNCTION 006f5070
	T2GUIRenderer* GetRenderer() const { return m_pRenderer; }

	// $deBlob: FUNCTION 006f5060
	void SetRenderer(T2GUIRenderer* pRenderer) { m_pRenderer = pRenderer; }

	T2GUIElement* GetRootElement() const { return m_pContext1->GetRootElement(); }

	// $deBlob: FUNCTION 006f5080
	void Tick(TFLOAT deltaTime)
	{
		if (m_pContext1)
		{
			m_pContext1->Tick(deltaTime);
		}
	}

	void Render();

	static T2GUIMaterial* CreateMaterial(const TCHAR* a_materialName);
	static void           DestroyMaterial(TMaterial* material);
	static TTexture*      GetTexture(const TCHAR* a_textureName);

private:
	static TMemoryHeap* s_pMemHeap;

private:
	T2GUIRenderer* m_pRenderer;
	T2GUIContext*  m_pContext1;
	T2GUIContext*  m_pContext2;
};

TOSHI_NAMESPACE_END
