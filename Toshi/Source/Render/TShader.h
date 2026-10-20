/**
 * @file TShader.h
 * Shader system for the Toshi engine
 * 
 * This file defines the shader system that provides:
 * - Shader program management
 * - Shader parameter handling
 * - Shader state management
 * - Platform-specific shader abstraction
 */

#pragma once
#include "TOrderTable.h"

TOSHI_NAMESPACE_START

class TShader : public TObject
{
public:
	TDECLARE_CLASS( TShader, TObject );

public:
	using State = TUINT32;
	enum State_ : State
	{
		State_None      = 0,
		State_Created   = BITFLAG( 0 ),
		State_Validated = BITFLAG( 1 ),
	};

	class TShaderList
	{
	public:
		using t_EnumerateCallback = void ( * )( TShader* a_pShader );

	public:
		TShaderList()
		    : m_pRoot( TNULL ) {}

		void AddShader( TShader* pShader );
		void RemoveShader( TShader* pShader );
		void EnumerateShaders( t_EnumerateCallback a_fnCallback );
		void FlushShaders();

		TShader* GetRootShader() { return m_pRoot; }

	private:
		TShader* m_pRoot;
	};

public:
	TShader();
	~TShader();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	/* Called before shader is destroyed. */
	virtual void OnDestroy();

	/* Additional call to flush something more? */
	virtual void Flush();

	/* Setup render state before rendering meshes. */
	virtual void StartFlush() = 0;

	/* Restore render states after all rendering is done. */
	virtual void EndFlush() = 0;

	/* Do some initial setups, f.e. create order tables. */
	virtual TBOOL Create();

	/* Make sure the shader is ready to use. */
	virtual TBOOL Validate();

	/* We don't need the shader, at least for now. */
	virtual void Invalidate();

	/* Prefix of the shader's resources (e.g. "ss_" for ASkinShader). */
	virtual const TCHAR* GetShaderPrefix() = 0;

	/* Why? I don't know */
	// $deBlob: FUNCTION 00567730
	virtual TBOOL TryInvalidate() { return TTRUE; }

	/* Why? I don't know */
	// $deBlob: FUNCTION 00567740
	virtual TBOOL TryValidate() { return TTRUE; }

	/* Renders the mesh stored in the render packet containing other useful info. */
	virtual void Render( TRenderPacket* a_pRenderPacket ) = 0;

	TBOOL IsCreated() const { return m_State & State_Created; }
	// $deBlob: FUNCTION 005696c0
	TUINT IsValidated() const { return TBOOL( ( m_State >> 1 ) & 1 ); }

	TShader* GetNextShader() { return m_pNextShader; }

public:
	static TShaderList sm_oShaderList;

private:
	State    m_State;       // 0x04
	TShader* m_pNextShader; // 0x08
};

TOSHI_NAMESPACE_END
