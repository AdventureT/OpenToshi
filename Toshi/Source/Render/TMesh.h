#pragma once
#include "TMaterial.h"

TOSHI_NAMESPACE_START

class TShader;

class TMesh : public TObject
{
public:
	TDECLARE_CLASS( TMesh, TObject );

public:
	using State = TUINT32;
	enum State_ : State
	{
		State_None      = 0,
		State_Created   = BITFLAG( 0 ),
		State_Validated = BITFLAG( 1 ),
		State_IsWater   = BITFLAG( 2 ),
		State_Glow      = BITFLAG( 3 )
	};

	// These flags can be used by the client classes
	using FLAG = TUINT32;
	enum FLAG_ : FLAG
	{
		FLAG_LOCKED = 0x8000
	};

public:
	TMesh()
	{
		m_pMaterial    = TNULL;
		m_pOwnerShader = TNULL;
		m_State        = State_None;
	}

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate();
	virtual void  Invalidate();
	virtual TBOOL Create();
	virtual TBOOL Render();
	virtual void  OnDestroy();

	// $Barnyard: FUNCTION 006ccd60
	void SetFlags( State a_eFlags, TBOOL a_bEnable )
	{
		if ( a_bEnable )
			m_State |= a_eFlags;
		else
			m_State &= ~a_eFlags;
	}

	void DestroyResource();

	TMaterial* GetMaterial() const;
	void       SetMaterial( TMaterial* a_pMaterial );

	TShader* GetOwnerShader();
	void     SetOwnerShader( TShader* a_pShader );

	TBOOL IsCreated() const;
	TBOOL IsValidated() const { return TBOOL( ( m_State >> 1 ) & 1 ); }
	TBOOL IsWater() const;

protected:
	~TMesh();

protected:
	TMaterial* m_pMaterial;    // 0x04
	TShader*   m_pOwnerShader; // 0x08
	State      m_State;        // 0x0C
};

TOSHI_NAMESPACE_END
