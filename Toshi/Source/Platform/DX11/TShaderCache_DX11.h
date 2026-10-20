#pragma once
#include "Toshi/TString8.h"

#include <d3d11.h>
#include <functional>
#include <map>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: identifies a vertex shader permutation (cache file "<flags>_<texgen23>_<texgen4>.vs")
//-----------------------------------------------------------------------------
struct VertexShaderId
{
	// The bits of uiTexGen4 above the texture coordinate generator
	enum Flags : TUINT32
	{
		Flags_HasVertices = 0x10000,
		Flags_Fog         = 0x20000,
	};

	TUINT32 uiFlags; // 0x00 permutation flags of the shader that owns the cache
	TUINT32 uiTexGen23; // 0x04
	TUINT32 uiTexGen4;  // 0x08

	TBOOL operator<( const VertexShaderId& a_rOther ) const
	{
		if ( uiFlags != a_rOther.uiFlags ) return uiFlags < a_rOther.uiFlags;
		if ( uiTexGen23 != a_rOther.uiTexGen23 ) return uiTexGen23 < a_rOther.uiTexGen23;
		return uiTexGen4 < a_rOther.uiTexGen4;
	}
}; // 0x0C

//-----------------------------------------------------------------------------
// Purpose: identifies a pixel shader permutation (cache file "<s0>_<s1>_<s2>_<s3>_<s4>_<flags>.ps")
//-----------------------------------------------------------------------------
struct PixelShaderId
{
	// The bits of uiFlags above the lighting function and the indirect lookups
	enum Flags : TUINT32
	{
		Flags_Fog = 0x40,
	};

	TINT8   aStages[ 5 ]; // 0x00 -1 when unused (the skin shader keeps its TEV stage modes in the first four)
	TUINT32 uiFlags;      // 0x08

	TBOOL operator<( const PixelShaderId& a_rOther ) const
	{
		for ( TINT i = 0; i < 5; i++ )
			if ( aStages[ i ] != a_rOther.aStages[ i ] ) return aStages[ i ] < a_rOther.aStages[ i ];

		return uiFlags < a_rOther.uiFlags;
	}
}; // 0x0C

//-----------------------------------------------------------------------------
// Purpose: compiled shader permutations of a shader, kept in a folder (Data\Shaders\Skin\, Data\Shaders\World\). The
// folder's hash.txt holds the hashes of the HLSL sources the files were compiled from: when they match the files are
// loaded, otherwise every permutation listed in the folder is recompiled from the sources.
//-----------------------------------------------------------------------------
class TShaderCache_DX11
{
public:
	using t_CompileVS         = std::function<ID3DBlob*( const VertexShaderId& )>;
	using t_CreateInputLayout = std::function<ID3D11InputLayout*( const VertexShaderId&, ID3DBlob* )>;
	using t_CompilePS         = std::function<ID3DBlob*( const PixelShaderId& )>;

	struct VertexShaderEntry
	{
		ID3D11VertexShader* pShader;      // 0x00
		ID3D11InputLayout*  pInputLayout; // 0x04
	};

public:
	// $deBlob: FUNCTION 00566a50
	TShaderCache_DX11()  = default;
	// $deBlob: FUNCTION 00566df0
	~TShaderCache_DX11() = default;

	void Create( const TCHAR* a_szPath, const TCHAR* a_szVSSource, const TCHAR* a_szPSSource, const t_CompileVS& a_fnCompileVS, const t_CreateInputLayout& a_fnCreateInputLayout, const t_CompilePS& a_fnCompilePS );

	TBOOL FindPixelShader( const PixelShaderId& a_rId, ID3D11PixelShader*& a_rOutShader );
	void  AddPixelShader( const PixelShaderId& a_rId, ID3D11PixelShader* a_pShader, const void* a_pBytecode, SIZE_T a_uiBytecodeLength );

	TBOOL FindVertexShader( const VertexShaderId& a_rId, ID3D11VertexShader*& a_rOutShader, ID3D11InputLayout*& a_rOutInputLayout );
	void  AddVertexShader( const VertexShaderId& a_rId, ID3D11VertexShader* a_pShader, ID3D11InputLayout* a_pInputLayout, const void* a_pBytecode, SIZE_T a_uiBytecodeLength );

	const TString8& GetPath() const { return m_strPath; }

	static TUINT32 HashString( const TCHAR* a_szString );

private:
	static TBOOL NeedsRecompiling( const TCHAR* a_szPath, TUINT32 a_uiVSHash, TUINT32 a_uiPSHash );
	static void  SaveHash( const TCHAR* a_szPath, TUINT32 a_uiVSHash, TUINT32 a_uiPSHash );
	static void  CreateDirectories( const TCHAR* a_szPath );
	static void  DeleteFiles( const TCHAR* a_szPath );

private:
	std::map<PixelShaderId, ID3D11PixelShader*>  m_PixelShaders;  // 0x00
	std::map<VertexShaderId, VertexShaderEntry> m_VertexShaders; // 0x08
	TString8                                     m_strPath;       // 0x10
}; // 0x1C

TOSHI_NAMESPACE_END
