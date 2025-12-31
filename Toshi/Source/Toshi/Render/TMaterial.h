#pragma once
#include "TTexture.h"

TOSHI_NAMESPACE_START

class TRegMaterial;
class T2Material;
class TShader;

class TMaterial : public TGenericClassDerived<TMaterial, TObject, "TMaterial", TMAKEVERSION(1, 0), TFALSE>
{
public:
	static constexpr size_t MAXTEXTURES   = 6;
	static constexpr size_t NAMESIZELIMIT = 31;

	typedef TUINT32 Flags;
	enum Flags_ : Flags
	{
		Flags_NULL         = 0,
		Flags_Unk1         = BITFIELD(0),
		Flags_Unk2         = BITFIELD(1),
		Flags_Unk3         = BITFIELD(2),
		Flags_Unk4         = BITFIELD(3),
		Flags_Created      = BITFIELD(4),
		Flags_NoDepthTest  = BITFIELD(27),
		Flags_AlphaUpdate  = BITFIELD(28),
		Flags_AlphaTexture = BITFIELD(31),
	};

public:
	// $deBlob: FUNCTION 00692350
	TMaterial()
	{
		m_Flags        = Flags_NULL;
		m_pOwnerShader = TNULL;
		m_iNumTex      = 0;
		m_pUVAnimation = TNULL;
		TUtil::MemClear(m_pTextures, sizeof(m_pTextures));
		m_pRegMaterial = TNULL;
		m_Name[0]      = '\0';
		m_pMetaMat     = TNULL;
	}

	virtual ~TMaterial() = default;

	// $deBlob: FUNCTION 00692300
	virtual void OnDestroy()
	{
		TASSERT(TTRUE == IsCreated());
		m_Flags &= ~Flags_Created;
		m_pUVAnimation = TNULL;
	}

	// $deBlob: FUNCTION 006923c0
	virtual TBOOL Create()
	{
		TASSERT(TFALSE == IsCreated());
		m_Flags |= Flags_Created;
		return TTRUE;
	}

	// $deBlob: FUNCTION 0054f580
	virtual void PreRender() {}

	// $deBlob: FUNCTION 0054f570
	virtual void PostRender() {}

	void SetOwnerShader(TShader* pShader)
	{
		TASSERT(TNULL == m_pOwnerShader);
		m_pOwnerShader = pShader;
	}

	TShader* GetOwnerShader() const { return m_pOwnerShader; }

	// $deBlob: FUNCTION 0054f5a0
	void SetTextureNum(TUINT32 a_iNumTextures)
	{
		TASSERT(a_iNumTextures <= MAXTEXTURES);
		m_iNumTex = a_iNumTextures;
	}

	// $deBlob: FUNCTION 0055e410
	void SetTexture(TUINT32 a_iStage, TTexture* a_pTexture)
	{
		TASSERT(a_iStage < MAXTEXTURES);
		m_pTextures[a_iStage] = a_pTexture;
	}

	TTexture* GetTexture(TUINT32 a_iStage = 0) const { return m_pTextures[a_iStage]; }

	// $deBlob: FUNCTION 00692440
	void SetName(const TCHAR* name)
	{
		if (name == TNULL)
		{
			m_Name[0] = '\0';
		}
		else
		{
			T2String8::Copy(m_Name, name, NAMESIZELIMIT);
		}
	}

	// $deBlob: FUNCTION 00692410
	void SetFlag(Flags flag, TBOOL set = TFALSE)
	{
		if (set)
		{
			if (flag & (Flags_Unk1 | Flags_Unk2 | Flags_Unk3))
			{
				m_Flags &= ~(Flags_Unk1 | Flags_Unk2 | Flags_Unk3);
			}

			m_Flags |= flag;
		}
		else
		{
			m_Flags &= ~flag;
		}
	}

	TBOOL IsCreated() const { return m_Flags & Flags_Created; }

	void SetRegMaterial(TRegMaterial* pRegMaterial) { m_pRegMaterial = pRegMaterial; }

	TRegMaterial* GetRegMaterial() const { return m_pRegMaterial; }

protected:
	Flags         m_Flags;                   // 0x04
	TShader*      m_pOwnerShader;            // 0x08
	TRegMaterial* m_pRegMaterial;            // 0x0C
	TUINT32       m_iNumTex;                 // 0x10
	TTexture*     m_pTextures[MAXTEXTURES];  // 0x14
	TCHAR         m_Name[NAMESIZELIMIT + 1]; // 0x2C
	void*         m_pUVAnimation;            // 0x4C
	T2Material*   m_pMetaMat;                // 0x50
};

TOSHI_NAMESPACE_END
