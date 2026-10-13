#include "ToshiPCH.h"
#include "TRenderParamTable.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// 0x00981ff0 (sorted: the handles are found with a binary search)
static const TCHAR* s_aV4ParamNames[] = {
	"",
	"tambcolour",
	"tdirlight0",
	"tdirlight0col",
	"tdirlight1",
	"tdirlight1col",
	"tdirlight2",
	"tdirlight2col",
	"teyeposms",
	"tlightscounts",
	"tpointlight0",
	"tpointlight0col",
	"tpointlight0params",
	"tpointlight1",
	"tpointlight1col",
	"tpointlight1params",
	"tpointlight2",
	"tpointlight2col",
	"tpointlight2params",
	"tpointlight3",
	"tpointlight3col",
	"tpointlight3params",
	"tshadowlightdir0",
	"tspotlight0",
	"tspotlight0col",
	"tspotlight0dir",
	"tspotlight0param",
	"tspotlight1",
	"tspotlight1col",
	"tspotlight1dir",
	"tspotlight1param",
	"tspotlight2",
	"tspotlight2col",
	"tspotlight2dir",
	"tspotlight2param",
	"{",
};

// 0x00982080
static const TCHAR* s_aM44ParamNames[] = { "", "tmodelview", "tmodelviewproj", "tmodelworld", "tproj", "{" };

// 0x00982098
static const TCHAR* s_aTexParamNames[] = { "", "tcubeenvmap", "tframebuftex", "tlightfalloffmap", "tplanarenvmap", "tshadowtex", "{" };

TSTATICASSERT( TARRAYSIZE( s_aV4ParamNames ) == TRenderParamTable::V4PARAM_NUMOF );
TSTATICASSERT( TARRAYSIZE( s_aM44ParamNames ) == TRenderParamTable::M44PARAM_NUMOF );
TSTATICASSERT( TARRAYSIZE( s_aTexParamNames ) == TRenderParamTable::TEXPARAM_NUMOF );

// $deBlob: FUNCTION 006b6300
TRenderParamTable::TRenderParamTable()
    : m_V4ParamNames( s_aV4ParamNames, V4PARAM_NUMOF )
    , m_M44ParamNames( s_aM44ParamNames, M44PARAM_NUMOF )
    , m_TexParamNames( s_aTexParamNames, TEXPARAM_NUMOF )
{
	m_iNumV4Params     = V4PARAM_NUMOF;
	m_iNumMatrixParams = M44PARAM_NUMOF;
	m_iNumTexParams    = TEXPARAM_NUMOF;

	m_Vectors  = TSTATICCAST( TVector4, TMemalign( 16, m_iNumV4Params * sizeof( TVector4 ) ) );
	m_Matrices = TSTATICCAST( TMatrix44, TMemalign( 16, m_iNumMatrixParams * sizeof( TMatrix44 ) ) );
	m_Textures = TSTATICCAST( TTexture*, TMemalign( 16, m_iNumTexParams * sizeof( TTexture* ) ) );

	for ( TINT i = 0; i < m_iNumV4Params; i++ )
		m_Vectors[ i ] = TVector4( 0.0f, 0.0f, 0.0f, 1.0f ); // TVector4::X0Y0Z0W1

	for ( TINT i = 0; i < m_iNumMatrixParams; i++ )
		m_Matrices[ i ] = TMatrix44::IDENTITY;

	for ( TINT i = 0; i < m_iNumTexParams; i++ )
		m_Textures[ i ] = TNULL;

	TASSERT( V4PARAM_AMBIENTCOLOUR == FindParameterV4Handle( "tambcolour" ) );
	TASSERT( V4PARAM_SHADOWLIGHTDIR0 == FindParameterV4Handle( "tshadowlightdir0" ) );
	TASSERT( V4PARAM_DIRLIGHT0 == FindParameterV4Handle( "tdirlight0" ) );
	TASSERT( V4PARAM_DIRLIGHT1 == FindParameterV4Handle( "tdirlight1" ) );
	TASSERT( V4PARAM_DIRLIGHT2 == FindParameterV4Handle( "tdirlight2" ) );
	TASSERT( V4PARAM_DIRLIGHT0COLOUR == FindParameterV4Handle( "tdirlight0col" ) );
	TASSERT( V4PARAM_DIRLIGHT1COLOUR == FindParameterV4Handle( "tdirlight1col" ) );
	TASSERT( V4PARAM_DIRLIGHT2COLOUR == FindParameterV4Handle( "tdirlight2col" ) );
	TASSERT( V4PARAM_EYEPOSMS == FindParameterV4Handle( "teyeposms" ) );

	SetParameterV4( V4PARAM_DIRLIGHT0, TVector4( 0.47f, -0.74f, 0.47f, 0.0f ) );
	SetParameterV4( V4PARAM_DIRLIGHT0COLOUR, TVector4( 1.0f, 1.0f, 0.73f, 0.0f ) );
	m_Vectors[ V4PARAM_DIRLIGHT1 ].w        = 0.0f;
	m_Vectors[ V4PARAM_DIRLIGHT2 ].w        = 0.0f;
	m_Vectors[ V4PARAM_DIRLIGHT1COLOUR ].w  = 0.0f;
	m_Vectors[ V4PARAM_DIRLIGHT2COLOUR ].w  = 0.0f;

	TASSERT( V4PARAM_SPOTLIGHT0 == FindParameterV4Handle( "tspotlight0" ) );
	TASSERT( V4PARAM_SPOTLIGHT1 == FindParameterV4Handle( "tspotlight1" ) );
	TASSERT( V4PARAM_SPOTLIGHT0COLOUR == FindParameterV4Handle( "tspotlight0col" ) );
	TASSERT( V4PARAM_SPOTLIGHT1COLOUR == FindParameterV4Handle( "tspotlight1col" ) );
	TASSERT( V4PARAM_SPOTLIGHT0DIR == FindParameterV4Handle( "tspotlight0dir" ) );
	TASSERT( V4PARAM_SPOTLIGHT1DIR == FindParameterV4Handle( "tspotlight1dir" ) );
	TASSERT( V4PARAM_SPOTLIGHT0PARAMS == FindParameterV4Handle( "tspotlight0param" ) );
	TASSERT( V4PARAM_SPOTLIGHT1PARAMS == FindParameterV4Handle( "tspotlight1param" ) );
	TASSERT( V4PARAM_POINTLIGHT0 == FindParameterV4Handle( "tpointlight0" ) );
	TASSERT( V4PARAM_POINTLIGHT1 == FindParameterV4Handle( "tpointlight1" ) );
	TASSERT( V4PARAM_POINTLIGHT2 == FindParameterV4Handle( "tpointlight2" ) );
	TASSERT( V4PARAM_POINTLIGHT3 == FindParameterV4Handle( "tpointlight3" ) );
	TASSERT( V4PARAM_POINTLIGHT0COLOUR == FindParameterV4Handle( "tpointlight0col" ) );
	TASSERT( V4PARAM_POINTLIGHT1COLOUR == FindParameterV4Handle( "tpointlight1col" ) );
	TASSERT( V4PARAM_POINTLIGHT2COLOUR == FindParameterV4Handle( "tpointlight2col" ) );
	TASSERT( V4PARAM_POINTLIGHT3COLOUR == FindParameterV4Handle( "tpointlight3col" ) );
	TASSERT( V4PARAM_POINTLIGHT0PARAMS == FindParameterV4Handle( "tpointlight0params" ) );
	TASSERT( V4PARAM_POINTLIGHT1PARAMS == FindParameterV4Handle( "tpointlight1params" ) );
	TASSERT( V4PARAM_POINTLIGHT2PARAMS == FindParameterV4Handle( "tpointlight2params" ) );
	TASSERT( V4PARAM_POINTLIGHT3PARAMS == FindParameterV4Handle( "tpointlight3params" ) );
	TASSERT( V4PARAM_LIGHTSCOUNTS == FindParameterV4Handle( "tlightscounts" ) );

	TASSERT( M44PARAM_MODELVIEW == FindParameterM44Handle( "tmodelview" ) );
	TASSERT( M44PARAM_MODELWORLD == FindParameterM44Handle( "tmodelworld" ) );
	TASSERT( M44PARAM_MODELVIEWPROJ == FindParameterM44Handle( "tmodelviewproj" ) );
	TASSERT( M44PARAM_PROJ == FindParameterM44Handle( "tproj" ) );

	TASSERT( TEXPARAM_CUBEENVMAP == FindParameterTexHandle( "tcubeenvmap" ) );
	TASSERT( TEXPARAM_FRAMEBUF == FindParameterTexHandle( "tframebuftex" ) );
	TASSERT( TEXPARAM_LIGHTFALLOFF == FindParameterTexHandle( "tlightfalloffmap" ) );
	TASSERT( TEXPARAM_PLANARENVMAP == FindParameterTexHandle( "tplanarenvmap" ) );
	TASSERT( TEXPARAM_SHADOWTEX == FindParameterTexHandle( "tshadowtex" ) );
}

// $deBlob: FUNCTION 006b6f40
TRenderParamTable::~TRenderParamTable()
{
	TFree( m_Vectors );
	TFree( m_Matrices );
	TFree( m_Textures );
}

// $deBlob: FUNCTION 006b6ff0
void TRenderParamTable::SetParameterV4( TINT a_Handle, const TVector4& a_rVector )
{
	TASSERT( a_Handle > 0 && a_Handle < m_iNumV4Params );
	m_Vectors[ a_Handle ] = a_rVector;
}

// $deBlob: FUNCTION 006b7050
void TRenderParamTable::SetParameterM44( TINT a_Handle, const TMatrix44& a_rMatrix )
{
	TASSERT( a_Handle >= 0 && a_Handle < m_iNumMatrixParams );
	m_Matrices[ a_Handle ] = a_rMatrix;
}

// $deBlob: FUNCTION 006b70d0
void TRenderParamTable::SetParameterTex( TINT a_Handle, TTexture* a_pTexture )
{
	TASSERT( a_Handle >= 0 && a_Handle < m_iNumTexParams );
	m_Textures[ a_Handle ] = a_pTexture;
}

TOSHI_NAMESPACE_END
