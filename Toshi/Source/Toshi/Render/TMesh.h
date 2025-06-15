#pragma once
#include "TMaterial.h"
#include "Toshi2/T2Flags.h"

TOSHI_NAMESPACE_START

class TShader;

class TMesh : public TGenericClassDerived<TMesh, TObject, "TMesh", TMAKEVERSION(1, 0), TFALSE>
{
public:
	enum class State : TUINT32
	{
		None      = 0,
		Created   = BITFIELD(0),
		Validated = BITFIELD(1),
	};

	struct TLockBuffer
	{
		TINT  Unknown;           // 0x00
		void* pVertexBufferData; // 0x08
		void* pIndexBufferData;  // 0x2C
	};

public:
	TMesh()
	{
		m_pOwnerShader = TNULL;
		m_pMaterial    = TNULL;
		m_State        = State::None;
	}

	virtual TBOOL Validate()
	{
		m_State.Set(State::Validated);
		return TTRUE;
	}

	virtual void Invalidate() { m_State.Unset(State::Validated); }

	virtual TBOOL Create()
	{
		m_State.Set(State::Created);
		return TTRUE;
	}

	virtual TBOOL Render() { return TTRUE; }

	virtual void OnDestroy() { m_State.Unset(State::Created); }

	void DestroyResource()
	{
		Invalidate();
		OnDestroy();
		delete this;
	}

	void SetMaterial(TMaterial* pMaterial) { m_pMaterial = pMaterial; }

	TMaterial* GetMaterial() const { return m_pMaterial; }

	void SetOwnerShader(TShader* pShader)
	{
		TASSERT(TNULL == m_pOwnerShader);
		m_pOwnerShader = pShader;
	}

	TShader* GetOwnerShader() { return m_pOwnerShader; }

	TBOOL IsCreated() const { return m_State.IsSet(State::Created); }

protected:
	TMaterial*     m_pMaterial;    // 0x04
	TShader*       m_pOwnerShader; // 0x08
	T2Flags<State> m_State;        // 0x0C
};

DEFINE_T2FLAGS(TMesh::State);

TOSHI_NAMESPACE_END
