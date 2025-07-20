#pragma once
#include "Toshi2/T2ResourceManager.h"

TOSHI_NAMESPACE_START

class TTexture;

class T2Material : public T2Resource
{
public:
	typedef enum
	{
		Flags_InMaterialManager = BITFIELD(0),
	} Flags;

	struct Texture
	{
		TTexture*    m_pTexture;
		const TCHAR* m_pName;
	};

	struct Pass
	{
		TUINT32  m_uiUnk1;
		TUINT32  m_uiBlendMode;
		TUINT32  m_uiUnk3;
		Texture* m_pTexture;
		TUINT32  m_uiUnk4;
		TUINT32  m_uiUnk5;
		TUINT32  m_uiUnk6;
		TUINT32  m_uiUnk7;
		TUINT32  m_uiUnk8;
		TUINT32  m_uiUnk9;
		TUINT32  m_uiUnk10;
	};

	using t_TRBLoadCallback   = void (*)(T2Material* pMetaMat, void* pData);
	using t_TRBUnloadCallback = void (*)(T2Material* pMetaMat, void* pData);

	friend class TMaterialManager;

public:
	void Init();
	void Deinit();

	TTexture* GetTexture(TINT a_iId) const
	{
		TASSERT(a_iId < m_iNumTex);
		return m_aTextures[a_iId].m_pTexture;
	}

	const TCHAR* GetTextureName(TINT a_iId) const
	{
		TASSERT(a_iId < m_iNumTex);
		return m_aTextures[a_iId].m_pName;
	}

	Pass* GetPassInfo(TINT a_iPass) const
	{
		TASSERT(a_iPass < m_iNumPasses);
		return &m_aPasses[a_iPass];
	}

	TUINT32 GetPassBlendMode(TINT a_iPass) const
	{
		TASSERT(a_iPass < m_iNumPasses);
		return m_aPasses[a_iPass].m_uiBlendMode;
	}

	TTexture* GetPassTexture(TINT a_iPass) const
	{
		TASSERT(a_iPass < m_iNumPasses);
		return m_aPasses[a_iPass].m_pTexture->m_pTexture;
	}

	const TCHAR* GetPassTextureName(TINT a_iPass) const
	{
		TASSERT(a_iPass < m_iNumPasses);
		return m_aPasses[a_iPass].m_pTexture->m_pName;
	}

	const TCHAR* GetMaterialName() const { return m_pMaterialName; }

	TINT GetNumTextures() const { return m_iNumTex; }

	TUINT32 GetNumPasses() const { return m_iNumPasses; }

	TBOOL HasFlag(Flags flag) { return HASFLAG(m_uiFlags & flag); }

private:
	void InitData();

	static void* ResourceCallback(void* pCustomData, TTRB* trb, TBOOL bCreated);

private:
	inline static t_TRBLoadCallback   sm_pTRBLoadCallback       = TNULL;
	inline static t_TRBUnloadCallback sm_pTRBUnloadCallback     = TNULL;
	inline static void*               sm_pTRBLoadCallbackData   = TNULL;
	inline static void*               sm_pTRBUnloadCallbackData = TNULL;

private:
	TUINT32      m_uiUnk1;
	TUINT32      m_uiFlags;
	const TCHAR* m_pMaterialName;
	TINT32       m_iNumTex;
	Texture*     m_aTextures;
	TUINT32      m_uiUnk2;
	TUINT32      m_uiUnk3;
	Pass*        m_aPasses;
	TINT32       m_iNumPasses;
	TUINT32      m_uiUnk4;
	T2Material*  m_pNext;
	T2Material*  m_pPrev;
};

DEFINE_T2FLAGS(T2Material::Flags);

class TMaterialManager : public TSingleton<TMaterialManager>
{
public:
	TMaterialManager() = default;

	void AddMaterial(T2Material* a_pMat);
	void RemoveMaterial(T2Material* a_pMat);

	T2Material* FindMaterial(const TCHAR* a_pName) const;

private:
	T2Material* m_pHeadMaterial = TNULL;
};

TOSHI_NAMESPACE_END
