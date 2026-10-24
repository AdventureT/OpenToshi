#include "ToshiPCH.h"
#include "T2GUI.h"

TOSHI_NAMESPACE_START

TMemoryHeap* T2GUI::s_pMemHeap = TNULL;

// $deBlob: FUNCTION 006f4ef0
T2GUI::T2GUI()
{
	m_pRenderer = TNULL;
	m_pContext1 = TNULL;
	m_pContext2 = TNULL;

	T2GUIContext* pContext = new T2GUIContext();
	m_pContext1            = pContext;
	m_pContext2            = pContext;
}

// $deBlob: FUNCTION 006f4f90
T2GUI::~T2GUI()
{
	if (m_pContext2)
	{
		delete m_pContext2;
	}
}

// $deBlob: FUNCTION 006f50a0
void T2GUI::Render()
{
	if (m_pContext1 && m_pRenderer)
	{
		m_pRenderer->BeginScene();
		m_pContext1->GetRootElement()->Render();
		m_pRenderer->EndScene();
	}
}

// $deBlob: FUNCTION 006f50e0
T2GUIMaterial* T2GUI::CreateMaterial(const TCHAR* a_materialName)
{
	TTexture* texture = GetTexture(a_materialName);

	T2GUIMaterial* mat;

	if (s_pMemHeap == TNULL)
	{
		mat = new T2GUIMaterial();
	}
	else
	{
		mat = new (s_pMemHeap) T2GUIMaterial();
	}

	mat->Create();
	mat->SetTexture(texture);

	return mat;
}

// $deBlob: FUNCTION 006f51b0
void T2GUI::DestroyMaterial(TMaterial* material)
{
	if (material != TNULL)
	{
		TASSERT(material->IsA(TGetClass(T2GUIMaterial)));
		material->OnDestroy();
	}
}

// $deBlob: FUNCTION 006f5220
TTexture* T2GUI::GetTexture(const TCHAR* a_textureName)
{
	TCHAR textureName[0x80];
	TStringManager::String8CopySafe(textureName, a_textureName, 0x80);
	TStringManager::String8ToLowerCase(textureName);

	TString8 texName = TString8(textureName);
	TINT     index   = texName.FindReverse('\\', -1);
	texName          = texName.Right(index);

	TTexture* texture = TTextureManager::GetSingleton()->FindTexture(textureName);
	return texture;
}

TOSHI_NAMESPACE_END
