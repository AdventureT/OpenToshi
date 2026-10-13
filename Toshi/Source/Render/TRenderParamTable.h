#pragma once
#include "Toshi/TStringTable.h"
#include "Math/TVector4.h"
#include "Math/TMatrix44.h"

TOSHI_NAMESPACE_START

class TTexture;

//-----------------------------------------------------------------------------
// de Blob: Source\Render\TRenderParamTable.cpp. Shader parameters shared by the renderer (lights, matrices,
// textures), created by TRender::TRender (0x30 bytes). The handles are indices of the (sorted) name tables.
//-----------------------------------------------------------------------------
class TRenderParamTable
{
public:
	enum V4PARAM
	{
		V4PARAM_AMBIENTCOLOUR = 1,
		V4PARAM_DIRLIGHT0,
		V4PARAM_DIRLIGHT0COLOUR,
		V4PARAM_DIRLIGHT1,
		V4PARAM_DIRLIGHT1COLOUR,
		V4PARAM_DIRLIGHT2,
		V4PARAM_DIRLIGHT2COLOUR,
		V4PARAM_EYEPOSMS,
		V4PARAM_LIGHTSCOUNTS,
		V4PARAM_POINTLIGHT0,
		V4PARAM_POINTLIGHT0COLOUR,
		V4PARAM_POINTLIGHT0PARAMS,
		V4PARAM_POINTLIGHT1,
		V4PARAM_POINTLIGHT1COLOUR,
		V4PARAM_POINTLIGHT1PARAMS,
		V4PARAM_POINTLIGHT2,
		V4PARAM_POINTLIGHT2COLOUR,
		V4PARAM_POINTLIGHT2PARAMS,
		V4PARAM_POINTLIGHT3,
		V4PARAM_POINTLIGHT3COLOUR,
		V4PARAM_POINTLIGHT3PARAMS,
		V4PARAM_SHADOWLIGHTDIR0,
		V4PARAM_SPOTLIGHT0,
		V4PARAM_SPOTLIGHT0COLOUR,
		V4PARAM_SPOTLIGHT0DIR,
		V4PARAM_SPOTLIGHT0PARAMS,
		V4PARAM_SPOTLIGHT1,
		V4PARAM_SPOTLIGHT1COLOUR,
		V4PARAM_SPOTLIGHT1DIR,
		V4PARAM_SPOTLIGHT1PARAMS,
		V4PARAM_SPOTLIGHT2,
		V4PARAM_SPOTLIGHT2COLOUR,
		V4PARAM_SPOTLIGHT2DIR,
		V4PARAM_SPOTLIGHT2PARAMS,
		V4PARAM_DUMMY,
		V4PARAM_NUMOF,
	};

	enum M44PARAM
	{
		M44PARAM_MODELVIEW = 1,
		M44PARAM_MODELVIEWPROJ,
		M44PARAM_MODELWORLD,
		M44PARAM_PROJ,
		M44PARAM_DUMMY,
		M44PARAM_NUMOF,
	};

	enum TEXPARAM
	{
		TEXPARAM_CUBEENVMAP = 1,
		TEXPARAM_FRAMEBUF,
		TEXPARAM_LIGHTFALLOFF,
		TEXPARAM_PLANARENVMAP,
		TEXPARAM_SHADOWTEX,
		TEXPARAM_DUMMY,
		TEXPARAM_NUMOF
	};

public:
	TRenderParamTable();
	~TRenderParamTable();

	TINT FindParameterV4Handle( const TCHAR* a_szName ) { return m_V4ParamNames.GetStringID( a_szName ); }
	TINT FindParameterM44Handle( const TCHAR* a_szName ) { return m_M44ParamNames.GetStringID( a_szName ); }
	TINT FindParameterTexHandle( const TCHAR* a_szName ) { return m_TexParamNames.GetStringID( a_szName ); }

	void SetParameterV4( TINT a_Handle, const TVector4& a_rVector );
	void SetParameterM44( TINT a_Handle, const TMatrix44& a_rMatrix );
	void SetParameterTex( TINT a_Handle, TTexture* a_pTexture );

	const TVector4&  GetParameterV4( TINT a_Handle ) const { return m_Vectors[ a_Handle ]; }
	const TMatrix44& GetParameterM44( TINT a_Handle ) const { return m_Matrices[ a_Handle ]; }
	TTexture*        GetParameterTex( TINT a_Handle ) const { return m_Textures[ a_Handle ]; }

private:
	TVector4*    m_Vectors;          // 0x00
	TMatrix44*   m_Matrices;         // 0x04
	TTexture**   m_Textures;         // 0x08
	TStringTable m_V4ParamNames;     // 0x0C
	TStringTable m_M44ParamNames;    // 0x14
	TStringTable m_TexParamNames;    // 0x1C
	TINT         m_iNumV4Params;     // 0x24
	TINT         m_iNumMatrixParams; // 0x28
	TINT         m_iNumTexParams;    // 0x2C
};

TOSHI_NAMESPACE_END
