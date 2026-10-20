#include "pch.h"
#include "AXUIBinkMovie_DX11.h"
#include "AMoviePlayer.h"

#include <Toshi/TString8.h>
#include <Toshi/T2String8.h>
#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIRenderer.h>
#include <XUI/TXUIVideo.h>
#include <Platform/DX11/TRenderContext_DX11.h>

#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static const TCHAR s_szPixelShader[] =
    "sampler2D YTexture : register( s0 );\t\t\t\t\tsampler2D UTexture : register( s1 );\t\t\t\t\tsampler2"
    "D VTexture : register( s2 );\t\t\t\t\tstruct PS_IN\t\t\t\t\t\t\t\t\t\t\t{\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
    "   float4 Position : POSITION;\t\t\t\t\t\t\t\tfloat2 UV : TEXCOORD0;\t\t\t\t\t\t\t\t};\t\t\t\t\t\t\t"
    "\t\t\t\t\t\t\tstatic const float3 offset = float3(-0.0625, -0.5, -0.5);\tstatic const float3 Rcoeff "
    "= float3(1.164, 0.000, 1.596);\tstatic const float3 Gcoeff = float3(1.164, -0.391, -0.813);static co"
    "nst float3 Bcoeff = float3(1.164, 2.018, 0.000);\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tfloat4 main( PS_IN In "
    ") : COLOR\t\t\t\t\t\t{\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tfloat3 yuv;\tyuv.r = tex2D( YTexture, In.UV ).a;"
    "\t\t\t\t\t\tyuv.g = tex2D( UTexture, In.UV ).a;\t\t\t\t\t\tyuv.b = tex2D( VTexture, In.UV ).a;\t\t\t"
    "\t\t\tyuv += offset;\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tfloat3 rgb;\t\t\t\t\t\t\t\t\t\t"
    "\t\trgb.r = dot(yuv, Rcoeff);\t\t\t\t\t\t\t\trgb.g = dot(yuv, Gcoeff);\t\t\t\t\t\t\t\trgb.b = dot(yu"
    "v, Bcoeff);\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\treturn float4(rgb, 1.0);\t\t\t\t\t\t\t}\t\t\t"
    "\t\t\t\t\t\t\t\t\t\t\t";

static const TCHAR s_szVertexShader[] =
    " float4x4 matWVP : register(c0);                                                             struct "
    "VS_IN                                                                                {              "
    "                                   float4 ObjPos   : POSITION;                   float2 UV\t\t  : TE"
    "XCOORD;\t\t\t    };                                                                                 "
    "         struct VS_OUT                                 {                                            "
    "     float4 ProjPos  : POSITION;                   float2 UV\t\t  : TEXCOORD0;              };      "
    "                                                                                    VS_OUT main( VS_"
    "IN In )                       {                                                 VS_OUT Out;         "
    "                          Out.ProjPos = mul( matWVP, In.ObjPos );       Out.UV = In.UV;\t\t\t\t\t\t "
    "       return Out;                               }                                            ";

// The vertices of the quad (TRIANGLESTRIP)
struct MovieVertex
{
	TFLOAT fX, fY, fZ;
	TFLOAT fU, fV;
};

// THEORAPLAY_Io::read: reads from the FILE in userdata
// $deBlob: FUNCTION 00553090
static long XUIMovieIoRead( THEORAPLAY_Io* a_pIo, void* a_pBuffer, long a_iBufferLength )
{
	FILE*  pFile   = TSTATICCAST( FILE, a_pIo->userdata );
	size_t uiCount = fread( a_pBuffer, 1, a_iBufferLength, pFile );

	if ( uiCount == 0 && ferror( pFile ) != 0 )
		return -1;

	return long( uiCount );
}

// THEORAPLAY_Io::close
// $deBlob: FUNCTION 005530d0
static void XUIMovieIoClose( THEORAPLAY_Io* a_pIo )
{
	fclose( TSTATICCAST( FILE, a_pIo->userdata ) );
}

// $deBlob: FUNCTION 005520f0
AXUIBinkMovie::AXUIBinkMovie()
{
	m_pVideo          = TNULL;
	m_iTexIndex       = 0;
	m_iTexturesWidth  = 0;
	m_iTexturesHeight = 0;
	m_pTheoraDecoder  = TNULL;
	m_pTheoraVideo    = TNULL;
	m_pChannel        = TNULL;
	m_pTheoraAudio    = TNULL;
	m_iAudioOffset    = 0;
	m_dPosition       = 0.0;
	m_uiUnk168        = 0;
	m_iUnk16C         = -1;
	m_fWidth          = 0.0f;
	m_fHeight         = 0.0f;
	m_bFrameReady     = TFALSE;
	m_bPaused         = TFALSE;
	m_bPauseAtEnd     = TFALSE;
	m_auiUnk17C[ 0 ]  = 0;
	m_auiUnk17C[ 1 ]  = 0;
	m_auiUnk17C[ 2 ]  = 0;
	m_auiUnk17C[ 3 ]  = 0;
	m_apTextures[ 0 ] = TNULL;
	m_apTextures[ 2 ] = TNULL;
	m_apTextures[ 4 ] = TNULL;
	m_apTextures[ 1 ] = TNULL;
	m_apTextures[ 3 ] = TNULL;
	m_apTextures[ 5 ] = TNULL;
	m_oIo.read        = XUIMovieIoRead;
	m_oIo.close       = XUIMovieIoClose;
	m_oIo.userdata    = TNULL;

	TRenderDX11* pRender = TRenderDX11::Interface();

	ID3DBlob* pPSBlob = TRenderDX11::CompileShader( s_szPixelShader, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader );
	pPSBlob->Release();

	ID3DBlob* pVSBlob = TRenderDX11::CompileShader( s_szVertexShader, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader );

	D3D11_INPUT_ELEMENT_DESC aInputDesc[ 2 ];
	aInputDesc[ 0 ].SemanticName         = "POSITION";
	aInputDesc[ 0 ].SemanticIndex        = 0;
	aInputDesc[ 0 ].Format               = DXGI_FORMAT_R32G32B32_FLOAT;
	aInputDesc[ 0 ].InputSlot            = 0;
	aInputDesc[ 0 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDesc[ 0 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDesc[ 0 ].InstanceDataStepRate = 0;
	aInputDesc[ 1 ].SemanticName         = "TEXCOORD";
	aInputDesc[ 1 ].SemanticIndex        = 0;
	aInputDesc[ 1 ].Format               = DXGI_FORMAT_R32G32_FLOAT;
	aInputDesc[ 1 ].InputSlot            = 0;
	aInputDesc[ 1 ].AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	aInputDesc[ 1 ].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	aInputDesc[ 1 ].InstanceDataStepRate = 0;

	pRender->GetDevice()->CreateInputLayout( aInputDesc, 2, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );
	pVSBlob->Release();
}

// $deBlob: FUNCTION 00552360
// $deBlob: FUNCTION 00553280
AXUIBinkMovie::~AXUIBinkMovie()
{
	AXUIBinkMovie::Close();

	for ( TINT i = 0; i < 2; i++ )
	{
		if ( m_apTextures[ i ] != TNULL )
		{
			m_apTextures[ i ]->Release();
			m_apTextures[ i ] = TNULL;
		}

		if ( m_apTextures[ i + 2 ] != TNULL )
		{
			m_apTextures[ i + 2 ]->Release();
			m_apTextures[ i + 2 ] = TNULL;
		}

		if ( m_apTextures[ i + 4 ] != TNULL )
		{
			m_apTextures[ i + 4 ]->Release();
			m_apTextures[ i + 4 ] = TNULL;
		}
	}

	if ( m_pVertexShader != TNULL )
	{
		m_pVertexShader->Release();
		m_pVertexShader = TNULL;
	}

	if ( m_pPixelShader != TNULL )
	{
		m_pPixelShader->Release();
		m_pPixelShader = TNULL;
	}

	if ( m_pInputLayout != TNULL )
	{
		m_pInputLayout->Release();
		m_pInputLayout = TNULL;
	}
}

// $deBlob: FUNCTION 00552460
TBOOL AXUIBinkMovie::Open( const TWCHAR* a_wszFileName, const TWCHAR* a_wszAudioFileName, TBOOL a_bPaused, TFLOAT a_fVolume )
{
	TStringManager::StringUnicodeToChar( m_szFileName, a_wszFileName, MAX_FILE_NAME );

	if ( a_wszAudioFileName == TNULL )
		m_szAudioFileName[ 0 ] = '\0';
	else
		TStringManager::StringUnicodeToChar( m_szAudioFileName, a_wszAudioFileName, MAX_FILE_NAME );

	return OpenMovie( m_szFileName, m_szAudioFileName, a_bPaused, a_fVolume );
}

// $deBlob: FUNCTION 005524c0
TBOOL AXUIBinkMovie::Unknown28()
{
	TASSERT( TFALSE );
	return TTRUE;
}

// $deBlob: FUNCTION 00552500
void AXUIBinkMovie::Close()
{

	if ( m_pTheoraDecoder == TNULL )
		return;

	if ( m_bHasAudioStream && m_pChannel != TNULL )
	{
		// NOTE: the channel is never created in retail
		FMOD::Sound* pSound = TNULL;
		m_pChannel->getCurrentSound( &pSound );
		m_pChannel->stop();

		for ( bool bIsPlaying = true; bIsPlaying; m_pChannel->isPlaying( &bIsPlaying ) )
			;

		pSound->release();
		m_pChannel = TNULL;
	}

	THEORAPLAY_stopDecode( m_pTheoraDecoder );
	m_pTheoraDecoder = TNULL;

	if ( m_pTheoraVideo != TNULL )
	{
		THEORAPLAY_freeVideo( m_pTheoraVideo );
		m_pTheoraVideo = TNULL;
	}

	if ( m_pTheoraAudio != TNULL )
	{
		THEORAPLAY_freeAudio( m_pTheoraAudio );
		m_pTheoraAudio = TNULL;
		m_iAudioOffset = 0;
	}

	m_uiUnk168 = 0;
	m_iUnk16C  = -1;
}

// $deBlob: FUNCTION 005525f0
void AXUIBinkMovie::SetPaused( TBOOL a_bPaused )
{
	if ( a_bPaused != m_bPaused )
	{
		if ( m_bHasAudioStream && m_pChannel != TNULL )
			m_pChannel->setPaused( a_bPaused );

		m_bPaused = a_bPaused;
	}
}

// $deBlob: FUNCTION 00552630
void AXUIBinkMovie::SetVolume( TFLOAT a_fVolume )
{
	if ( m_bHasAudioStream && m_pChannel != TNULL )
	{
		TMath::Clip( a_fVolume, 0.0f, 1.0f );
		m_pChannel->setVolume( a_fVolume );
	}
}

// $deBlob: FUNCTION 00552680
TBOOL AXUIBinkMovie::IsClosed()
{
	if ( m_pTheoraDecoder == TNULL )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 00552690
void AXUIBinkMovie::Update( TFLOAT a_fDeltaTime )
{

	if ( m_pTheoraDecoder == TNULL || m_bPaused )
		return;

	TASSERT( m_pTheoraDecoder && m_pTheoraVideo );

	m_dPosition += TFLOAT( a_fDeltaTime * 1000.0f );

	// The audio packets are thrown away (the FMOD channel is never created)
	if ( m_bHasAudioStream && m_pChannel == TNULL )
	{
		for ( const THEORAPLAY_AudioPacket* pAudio = THEORAPLAY_getAudio( m_pTheoraDecoder ); pAudio != TNULL; pAudio = THEORAPLAY_getAudio( m_pTheoraDecoder ) )
			THEORAPLAY_freeAudio( pAudio );
	}

	TUINT32 uiPosition = 0;

	if ( m_pChannel != TNULL )
	{
		m_pChannel->getPosition( &uiPosition, FMOD_TIMEUNIT_MS );
	}
	else
	{
		uiPosition = TUINT32( m_dPosition );
	}

	const THEORAPLAY_VideoFrame* pVideo = m_pTheoraVideo;

	if ( TINT( uiPosition - pVideo->playms ) > m_iFrameMS )
	{
		while ( TTRUE )
		{
			THEORAPLAY_freeVideo( pVideo );
			TINT bIsDecoding = THEORAPLAY_isDecoding( m_pTheoraDecoder );
			m_pTheoraVideo   = THEORAPLAY_getVideo( m_pTheoraDecoder );

			while ( m_pTheoraVideo == TNULL && bIsDecoding != 0 )
			{
				bIsDecoding    = THEORAPLAY_isDecoding( m_pTheoraDecoder );
				m_pTheoraVideo = THEORAPLAY_getVideo( m_pTheoraDecoder );
			}

			pVideo = m_pTheoraVideo;

			if ( pVideo == TNULL )
				break;

			if ( TINT( uiPosition - pVideo->playms ) <= m_iFrameMS )
			{
				UploadFrame();
				return;
			}
		}

		// The end of the movie
		if ( m_bPauseAtEnd )
			SetPaused( TTRUE );

		m_pVideo->OnMovieFinished();
		m_uiUnk168 = 0;

		if ( m_pVideo->IsLooping() )
			Restart( 0 );
	}

	UploadFrame();
}

// $deBlob: FUNCTION 00552850
void AXUIBinkMovie::Render( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	// The full-screen movies are drawn instead
	if ( AMoviePlayer::GetSingleton()->IsMoviePlaying() || AMoviePlayer::GetSingleton()->IsMoviePaused() || !m_bFrameReady )
		return;

	TXUI::GetSingleton()->GetRenderer()->UpdateTransform();

	TRenderDX11*         pRender        = TRenderDX11::Interface();
	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();

	pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader, TNULL, 0 );
	pDeviceContext->PSSetShaderResources( 0, 1, &m_apTextures[ m_iTexIndex ] );
	pRender->SetSamplerState( 0, 1, TRUE );
	pDeviceContext->PSSetShaderResources( 1, 1, &m_apTextures[ m_iTexIndex + 2 ] );
	pRender->SetSamplerState( 1, 1, TRUE );
	pDeviceContext->PSSetShaderResources( 2, 1, &m_apTextures[ m_iTexIndex + 4 ] );
	pRender->SetSamplerState( 2, 1, TRUE );

	// World view projection = ModelView * Projection (DirectXMath's XMMatrixMultiply)
	TRenderContextDX11* pRenderContext = TSTATICCAST( TRenderContextDX11, pRender->GetCurrentContext() );
	TMatrix44           oProjection    = pRenderContext->GetProjectionMatrix();
	TMatrix44           oModelView     = pRenderContext->GetModelViewMatrix();

	TMatrix44 oWorldViewProjection;
	DirectX::XMStoreFloat4x4(
	    TREINTERPRETCAST( DirectX::XMFLOAT4X4*, &oWorldViewProjection ),
	    DirectX::XMMatrixMultiply(
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oModelView ) ),
	        DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &oProjection ) )
	    )
	);
	pRender->SetVec4InVSBuffer( 0, &oWorldViewProjection, 4 );

	pDeviceContext->IASetInputLayout( m_pInputLayout );

	// A quad of the element's size centred on its position
	TFLOAT fLeft   = a_fWidth * -0.5f;
	TFLOAT fTop    = a_fHeight * -0.5f;
	TFLOAT fRight  = fLeft + a_fWidth;
	TFLOAT fBottom = fTop + a_fHeight;

	MovieVertex aVertices[ 4 ] = {
		{ fLeft, fTop, 0.0f, 0.0f, 0.0f },
		{ fRight, fTop, 0.0f, 1.0f, 0.0f },
		{ fLeft, fBottom, 0.0f, 0.0f, 1.0f },
		{ fRight, fBottom, 0.0f, 1.0f, 1.0f },
	};

	pRender->DrawNonIndexedImmediate( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, aVertices, 4, sizeof( MovieVertex ), 0 );
}

// $deBlob: FUNCTION 00552b70
TBOOL AXUIBinkMovie::Restart( TINT a_iUnk )
{
	m_iUnk16C = a_iUnk;
	return OpenMovie( m_szFileName, m_szAudioFileName, TFALSE, 1.0f );
}

// $deBlob: FUNCTION 00552ba0
TFLOAT AXUIBinkMovie::GetWidth()
{
	return m_fWidth;
}

// $deBlob: FUNCTION 00552bb0
TFLOAT AXUIBinkMovie::GetHeight()
{
	return m_fHeight;
}

// $deBlob: FUNCTION 00552be0
TBOOL AXUIBinkMovie::OpenMovie( const TCHAR* a_szFileName, const TCHAR* a_szAudioFileName, TBOOL a_bPaused, TFLOAT a_fVolume )
{
	Close();

	// The file name without its folder, in lower case
	TString8 strFileName( a_szFileName );
	TINT     iStart = strFileName.FindReverse( '\\', -1 ) + 1;

	if ( iStart == 0 )
		iStart = strFileName.FindReverse( '/', -1 ) + 1;

	strFileName = strFileName.Right( strFileName.Length() - iStart );
	strFileName.MakeLower();

	TCHAR szPath[ 256 ];
	T2String8::Format( szPath, "data/movies/%s.ogv", strFileName.GetString() );

#ifdef DEBLOB_COMMUNITY_PATCH
	m_oIo.userdata = AMoviePlayer::OpenFile( szPath );

	if ( m_oIo.userdata == TNULL )
		m_oIo.userdata = AMoviePlayer::OpenFile( m_szFileName );
#else
	m_oIo.userdata = fopen( szPath, "rb" );

	if ( m_oIo.userdata == TNULL )
		m_oIo.userdata = fopen( m_szFileName, "rb" );
#endif // DEBLOB_COMMUNITY_PATCH

	if ( m_oIo.userdata == TNULL )
		return TFALSE;

	m_pTheoraDecoder = ATheoraIo_StartDecode( &m_oIo, 30, THEORAPLAY_VIDFMT_IYUV );

	if ( m_pTheoraDecoder == TNULL )
		return TFALSE;

	while ( THEORAPLAY_isInitialized( m_pTheoraDecoder ) == 0 )
		;

	if ( THEORAPLAY_hasVideoStream( m_pTheoraDecoder ) == 0 )
	{
		THEORAPLAY_stopDecode( m_pTheoraDecoder );
		m_pTheoraDecoder = TNULL;
		return TFALSE;
	}

	m_bHasAudioStream = THEORAPLAY_hasAudioStream( m_pTheoraDecoder ) != 0;

	const THEORAPLAY_VideoFrame*  pVideo = TNULL;
	const THEORAPLAY_AudioPacket* pAudio = TNULL;

	// NOTE: the first audio packet is never freed
	do
	{
		do
		{
			if ( pVideo == TNULL )
				pVideo = THEORAPLAY_getVideo( m_pTheoraDecoder );

			if ( m_bHasAudioStream && pAudio == TNULL )
				pAudio = THEORAPLAY_getAudio( m_pTheoraDecoder );

		} while ( pVideo == TNULL );

	} while ( m_bHasAudioStream && pAudio == TNULL );

	m_iFrameMS = ( pVideo->fps == 0.0 ) ? 0 : TINT( TUINT32( 1000.0 / pVideo->fps ) );

	TBOOL bSizeChanged = m_iTexturesWidth != TINT( pVideo->width ) || m_iTexturesHeight != TINT( pVideo->height );
	m_iTexturesWidth   = pVideo->width;
	m_iTexturesHeight  = pVideo->height;

	if ( bSizeChanged )
		CreateTextures();

	m_pTheoraVideo = pVideo;

	// Wait until 30 frames are decoded or the decoder stops adding frames
	TUINT32 uiPrevAvailable = 0;

	for ( TUINT32 uiAvailable = THEORAPLAY_availableVideo( m_pTheoraDecoder ); uiAvailable < 30; uiAvailable = THEORAPLAY_availableVideo( m_pTheoraDecoder ) )
	{
		if ( uiAvailable == uiPrevAvailable )
			break;

		uiPrevAvailable = uiAvailable;
	}

	SetPaused( a_bPaused );
	m_uiUnk168 = 0;
	SetVolume( a_fVolume );
	m_dPosition = 0.0;
	return TTRUE;
}

// $deBlob: FUNCTION 00552ef0
void AXUIBinkMovie::CreateTextures()
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	for ( TINT i = 0; i < 2; i++ )
	{
		if ( m_apTextures[ i ] != TNULL )
		{
			m_apTextures[ i ]->Release();
			m_apTextures[ i ] = TNULL;
		}

		if ( m_apTextures[ i + 2 ] != TNULL )
		{
			m_apTextures[ i + 2 ]->Release();
			m_apTextures[ i + 2 ] = TNULL;
		}

		if ( m_apTextures[ i + 4 ] != TNULL )
		{
			m_apTextures[ i + 4 ]->Release();
			m_apTextures[ i + 4 ] = TNULL;
		}

		m_apTextures[ i ]     = pRender->CreateTexture( m_iTexturesWidth, m_iTexturesHeight, DXGI_FORMAT_A8_UNORM, TNULL, 0, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 1 );
		m_apTextures[ i + 2 ] = pRender->CreateTexture( m_iTexturesWidth >> 1, m_iTexturesHeight >> 1, DXGI_FORMAT_A8_UNORM, TNULL, 0, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 1 );
		m_apTextures[ i + 4 ] = pRender->CreateTexture( m_iTexturesWidth >> 1, m_iTexturesHeight >> 1, DXGI_FORMAT_A8_UNORM, TNULL, 0, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 1 );
	}
}

// $deBlob: FUNCTION 00552fe0
void AXUIBinkMovie::UploadFrame()
{
	const THEORAPLAY_VideoFrame* pVideo = m_pTheoraVideo;

	if ( pVideo == TNULL )
		return;

	TRenderDX11* pRender = TRenderDX11::Interface();

	// IYUV: the Y plane, then the U and V planes at half the size
	TUINT32       uiHalfWidth         = pVideo->width >> 1;
	TUINT32       uiWidth             = pVideo->width;
	TUINT32       uiQuarterPixelCount = uiHalfWidth * ( pVideo->height >> 1 );
	TUINT32       uiPixelCount        = uiWidth * pVideo->height;
	const TUINT8* pUV                 = pVideo->pixels + uiPixelCount;

	pRender->CopyDataToTexture( m_apTextures[ m_iTexIndex ], uiPixelCount, pVideo->pixels, uiWidth );
	pRender->CopyDataToTexture( m_apTextures[ m_iTexIndex + 2 ], uiQuarterPixelCount, pUV, uiHalfWidth );
	pRender->CopyDataToTexture( m_apTextures[ m_iTexIndex + 4 ], uiQuarterPixelCount, pUV + uiQuarterPixelCount, uiHalfWidth );
	m_bFrameReady = TTRUE;
}
