#include "pch.h"
#include "AMoviePlayer_DX11.h"

#include <Toshi/TUtil.h>
#include <Toshi/T2String8.h>
#include "Options/AOptions.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The full-screen quad (TRIANGLESTRIP, float2 positions)
static const TFLOAT s_afScreenMesh[ 8 ] = {
	-1.0f, -1.0f,
	-1.0f, 1.0f,
	1.0f, -1.0f,
	1.0f, 1.0f
};

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
    " struct VS_OUT                                 {                                                 flo"
    "at4 Position : POSITION;                   float2 UV : TEXCOORD0;                    };             "
    "                                                                             VS_OUT main( float2 Pos"
    "ition : POSITION )     {                                                 VS_OUT Out;                "
    "                   Out.Position = float4(Position, 0.0f, 1.0f);       Out.UV = Position.xy * float2("
    "0.5f, -0.5f) + 0.5f;           return Out;                               }                          "
    "                  ";

// THEORAPLAY_Io::read: reads from the FILE in userdata
// $deBlob: FUNCTION 00551f10
static long MovieIoRead( THEORAPLAY_Io* a_pIo, void* a_pBuffer, long a_iBufferLength )
{
	FILE*  pFile   = TSTATICCAST( FILE, a_pIo->userdata );
	size_t uiCount = fread( a_pBuffer, 1, a_iBufferLength, pFile );

	if ( uiCount == 0 && ferror( pFile ) != 0 )
		return -1;

	return long( uiCount );
}

// THEORAPLAY_Io::close
// $deBlob: FUNCTION 00551f50
static void MovieIoClose( THEORAPLAY_Io* a_pIo )
{
	fclose( TSTATICCAST( FILE, a_pIo->userdata ) );
}

// The FMOD stream's read callback (FMOD_CREATESOUNDEXINFO::pcmreadcallback, the FMOD stream thread)
// $deBlob: FUNCTION 00551f70
static FMOD_RESULT F_CALLBACK pcmreadcallback( FMOD_SOUND* a_pSound, void* a_pData, unsigned int a_uiDataLength )
{
	ADX11MoviePlayer* pMoviePlayer = TNULL;
	TREINTERPRETCAST( FMOD::Sound*, a_pSound )->getUserData( TREINTERPRETCAST( void**, &pMoviePlayer ) );
	pMoviePlayer->ReadBuffer( a_pData, a_uiDataLength );

	return FMOD_OK;
}

// FMOD_CREATESOUNDEXINFO::pcmsetposcallback
// $deBlob: FUNCTION 00551fa0
static FMOD_RESULT F_CALLBACK pcmsetposcallback( FMOD_SOUND* a_pSound, int a_iSubSound, unsigned int a_uiPosition, FMOD_TIMEUNIT a_ePosType )
{
	ADX11MoviePlayer* pMoviePlayer = TNULL;
	TREINTERPRETCAST( FMOD::Sound*, a_pSound )->getUserData( TREINTERPRETCAST( void**, &pMoviePlayer ) );

	return FMOD_OK;
}

// $deBlob: FUNCTION 00551150
ADX11MoviePlayer* ADX11MoviePlayer::Create()
{
	return new ADX11MoviePlayer();
}

// $deBlob: FUNCTION 005511c0
ADX11MoviePlayer::ADX11MoviePlayer()
{
	m_uiUnk14          = 0;
	m_iTexIndex        = 0;
	m_uiTexturesWidth  = 0;
	m_uiTexturesHeight = 0;
	m_pTheoraDecoder   = TNULL;
	m_pTheoraVideo     = TNULL;
	m_pChannel         = TNULL;
	m_pTheoraAudio     = TNULL;
	m_iAudioOffset     = 0;
	m_dPosition        = 0.0;
	m_apTextures[ 0 ]  = TNULL;
	m_apTextures[ 2 ]  = TNULL;
	m_apTextures[ 4 ]  = TNULL;
	m_apTextures[ 1 ]  = TNULL;
	m_apTextures[ 3 ]  = TNULL;
	m_apTextures[ 5 ]  = TNULL;
	m_oIo.read         = MovieIoRead;
	m_oIo.close        = MovieIoClose;
	m_oIo.userdata     = TNULL;
}

// $deBlob: FUNCTION 00551fd0
// $deBlob: FUNCTION 00551270
ADX11MoviePlayer::~ADX11MoviePlayer()
{
}

// $deBlob: FUNCTION 00551280
TBOOL ADX11MoviePlayer::PlayMovie( const TCHAR* a_szFileName, TUINT32 a_uiSoundChannel, PlayFlags a_uiFlags )
{
	if ( IsMoviePlaying() == TTRUE )
	{
		StopMovieImpl();
		TUtil::MemSet( m_szCurrentFileName, 0, MAX_FILE_NAME );
	}

	if ( a_szFileName == TNULL )
		return TFALSE;

	TCHAR szPath[ 256 ];
	strcpy( m_szCurrentFileName, a_szFileName );
	sprintf( szPath, "Data\\Movies\\%s.bik.ogv", m_szCurrentFileName );
	m_uiUnk148 = 0;

#ifdef DEBLOB_COMMUNITY_PATCH
	m_oIo.userdata = AMoviePlayer::OpenFile( szPath );

	if ( m_oIo.userdata == TNULL )
		m_oIo.userdata = AMoviePlayer::OpenFile( m_szCurrentFileName );
#else
	m_oIo.userdata = fopen( szPath, "rb" );

	if ( m_oIo.userdata == TNULL )
		m_oIo.userdata = fopen( m_szCurrentFileName, "rb" );
#endif // DEBLOB_COMMUNITY_PATCH

	if ( m_oIo.userdata != TNULL )
	{
		m_pTheoraDecoder = ATheoraIo_StartDecode( &m_oIo, 30, THEORAPLAY_VIDFMT_IYUV );

		if ( m_pTheoraDecoder != TNULL )
		{
			while ( THEORAPLAY_isInitialized( m_pTheoraDecoder ) == 0 )
				;

			if ( THEORAPLAY_hasVideoStream( m_pTheoraDecoder ) != 0 )
			{
				m_bHasAudioStream = THEORAPLAY_hasAudioStream( m_pTheoraDecoder ) != 0;

				const THEORAPLAY_VideoFrame*  pVideo = TNULL;
				const THEORAPLAY_AudioPacket* pAudio = TNULL;

				// NOTE: the first audio packet is only used for the format of the FMOD stream and never freed
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

				if ( m_uiTexturesWidth != pVideo->width || m_uiTexturesHeight != pVideo->height )
				{
					ReleaseTextures();
					CreateTextures( pVideo->width, pVideo->height );
				}

				m_uiTexturesWidth  = pVideo->width;
				m_pTheoraVideo     = pVideo;
				m_uiTexturesHeight = pVideo->height;

				// Wait until 30 frames are decoded or the decoder stops adding frames
				TUINT32 uiAvailable     = THEORAPLAY_availableVideo( m_pTheoraDecoder );
				TUINT32 uiPrevAvailable = 0;

				while ( uiAvailable < 30 && uiAvailable != uiPrevAvailable )
				{
					TUINT32 uiNewAvailable = THEORAPLAY_availableVideo( m_pTheoraDecoder );
					uiPrevAvailable        = uiAvailable;
					uiAvailable            = uiNewAvailable;
				}

				if ( m_bHasAudioStream )
				{
					// The FMOD stream of the audio, fed by pcmreadcallback
					FMOD::System* pSystem = TSound::GetSingleton()->GetSystem();

					FMOD_CREATESOUNDEXINFO oSoundInfo;
					memset( &oSoundInfo, 0, sizeof( oSoundInfo ) );
					oSoundInfo.cbsize            = sizeof( oSoundInfo );
					oSoundInfo.length            = 0xFFFFFFFF;
					oSoundInfo.numchannels       = pAudio->channels;
					oSoundInfo.defaultfrequency  = pAudio->freq;
					oSoundInfo.format            = FMOD_SOUND_FORMAT_PCMFLOAT;
					oSoundInfo.userdata          = this;
					oSoundInfo.decodebuffersize  = ( pAudio->freq * pAudio->channels ) / 16;
					oSoundInfo.pcmreadcallback   = pcmreadcallback;
					oSoundInfo.pcmsetposcallback = pcmsetposcallback;

					FMOD::Sound* pSound;

					if ( pSystem->createSound( TNULL, FMOD_LOOP_NORMAL | FMOD_2D | FMOD_SOFTWARE | FMOD_CREATESTREAM | FMOD_OPENUSER, &oSoundInfo, &pSound ) == FMOD_OK &&
					     pSystem->playSound( FMOD_CHANNEL_FREE, pSound, true, &m_pChannel ) != FMOD_OK )
					{
						m_pChannel = TNULL;
						pSound->release();
						pSound = TNULL;
					}

					if ( m_pChannel != TNULL )
					{
						m_pChannel->setPriority( 0 );
						m_pChannel->setVolume( AOptions::GetSingleton()->GetMusicVolume() );
						m_pChannel->setPaused( false );
					}
				}

				Mute( ( a_uiFlags >> 1 ) & 1 );
				m_bIsMovieLooping = a_uiFlags & 1;
				m_bIsHidden       = TFALSE;
				m_bIsPlaying      = TTRUE;
				m_bIsPaused       = TFALSE;
				m_dPosition       = 0.0;
				return TTRUE;
			}

			THEORAPLAY_stopDecode( m_pTheoraDecoder );
			m_pTheoraDecoder = TNULL;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00551610
void ADX11MoviePlayer::StopMovie()
{
	StopMovieImpl();
	ThrowEvent( AMovieEvent::Type_Stopped );
}

// $deBlob: FUNCTION 00551630
void ADX11MoviePlayer::PauseMovie( TBOOL a_bPause )
{
	if ( a_bPause != m_bIsPaused )
	{
		if ( m_bIsPlaying && m_pChannel != TNULL )
			m_pChannel->setPaused( a_bPause );

		m_bIsPaused = a_bPause;
	}
}

// $deBlob: FUNCTION 00551660
void ADX11MoviePlayer::Mute( TBOOL a_bMute )
{
	if ( m_bHasAudioStream && m_pChannel != TNULL )
	{
		TFLOAT fVolume = AOptions::GetSingleton()->GetMusicVolume();
		m_pChannel->setVolume( a_bMute ? 0.0f : fVolume );
		m_bIsMuted = a_bMute;
		return;
	}

	m_bIsMuted = a_bMute;
}

// $deBlob: FUNCTION 005516d0
void ADX11MoviePlayer::OnCreate()
{
	CompileShader();
	AMoviePlayer::OnCreate();
}

// $deBlob: FUNCTION 005516e0
void ADX11MoviePlayer::OnDestroy()
{
	ReleaseTextures();
	ReleaseShaders();
	AMoviePlayer::OnDestroy();
}

// $deBlob: FUNCTION 00551740
TBOOL ADX11MoviePlayer::OnUpdate( TFLOAT a_fDeltaTime )
{

	if ( IsMoviePlaying() == TTRUE && IsMoviePaused() == TFALSE )
	{
		TASSERT( m_pTheoraDecoder && m_pTheoraVideo );

		m_dPosition += a_fDeltaTime * 1000.0f;

		// Without the FMOD stream the audio packets are thrown away
		if ( m_bHasAudioStream && m_pChannel == TNULL )
		{
			const THEORAPLAY_AudioPacket* pAudio = THEORAPLAY_getAudio( m_pTheoraDecoder );

			while ( pAudio != TNULL )
			{
				THEORAPLAY_freeAudio( pAudio );
				pAudio = THEORAPLAY_getAudio( m_pTheoraDecoder );
			}
		}

		TUINT32 uiPosition = 0;

		if ( m_pChannel == TNULL )
		{
			uiPosition = TUINT32( m_dPosition );
		}
		else
		{
			m_pChannel->getPosition( &uiPosition, FMOD_TIMEUNIT_MS );
		}

		if ( uiPosition == 0 )
			uiPosition = TUINT32( m_dPosition );

		const THEORAPLAY_VideoFrame* pVideo = m_pTheoraVideo;

		if ( m_iFrameMS < TINT( uiPosition - pVideo->playms ) )
		{
			do
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
				{
					// The end of the movie
					if ( m_bIsMovieLooping == TTRUE )
					{
						ThrowEvent( AMovieEvent::Type_Looping );
						PlayFlags uiFlags = m_bIsMuted ? PlayFlags_Muted : PlayFlags_None;
						PlayMovie( m_szCurrentFileName, 0, uiFlags | ( m_bIsMovieLooping != TFALSE ) );
					}
					else
					{
						StopMovieImpl();
						ThrowEvent( AMovieEvent::Type_Finished );
					}

					break;
				}

			} while ( m_iFrameMS < TINT( uiPosition - pVideo->playms ) );
		}
	}

	return AMoviePlayer::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 00551930
void ADX11MoviePlayer::OnRender( TFLOAT a_fDeltaTime )
{
	if ( IsMoviePlaying() != TTRUE )
		return;

	TRenderDX11* pRender = TRenderDX11::Interface();

	if ( IsMoviePaused() != TFALSE || m_pTheoraVideo == TNULL )
		return;

	const THEORAPLAY_VideoFrame* pVideo = m_pTheoraVideo;

	TUINT32       uiWidth             = pVideo->width;
	TUINT32       uiPixelCount        = pVideo->height * uiWidth;
	TUINT32       uiHalfWidth         = uiWidth >> 1;
	TUINT32       uiQuarterPixelCount = ( pVideo->height >> 1 ) * uiHalfWidth;
	const TUINT8* pUV                 = pVideo->pixels + uiPixelCount;

	// IYUV: the Y plane, then the U and V planes at half the size
	pRender->CopyDataToTexture( m_apTextures[ m_iTexIndex ], uiPixelCount, pVideo->pixels, uiWidth );
	pRender->CopyDataToTexture( m_apTextures[ m_iTexIndex + 2 ], uiQuarterPixelCount, pUV, uiHalfWidth );
	pRender->CopyDataToTexture( m_apTextures[ m_iTexIndex + 4 ], uiQuarterPixelCount, pUV + uiQuarterPixelCount, uiHalfWidth );

	pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
	pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO );

	ID3D11DeviceContext* pDeviceContext = pRender->GetDeviceContext();
	pDeviceContext->VSSetShader( m_pVertexShader, TNULL, 0 );
	pDeviceContext->PSSetShader( m_pPixelShader, TNULL, 0 );
	pDeviceContext->PSSetShaderResources( 0, 1, &m_apTextures[ m_iTexIndex ] );
	pRender->SetSamplerState( 0, 1, TRUE );
	pDeviceContext->PSSetShaderResources( 1, 1, &m_apTextures[ m_iTexIndex + 2 ] );
	pRender->SetSamplerState( 1, 1, TRUE );
	pDeviceContext->PSSetShaderResources( 2, 1, &m_apTextures[ m_iTexIndex + 4 ] );
	pRender->SetSamplerState( 2, 1, TRUE );
	pDeviceContext->IASetInputLayout( m_pInputLayout );

	// Cull mode: D3D11_CULL_NONE
	TUINT8& rRasterizerFlags = TREINTERPRETCAST( TUINT8*, &pRender->GetCurrentRasterizer().uiFlags )[ 0 ];
	rRasterizerFlags         = ( rRasterizerFlags & 0xF7 ) | 0x04;

	pRender->DrawNonIndexed( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, m_pVertexBuffer, 4, 8, 0, 0 );
}

// $deBlob: FUNCTION 00551ab0
void ADX11MoviePlayer::ReadBuffer( void* a_pData, TUINT32 a_uiDataLength )
{
	TCHAR* pData = TSTATICCAST( TCHAR, a_pData );

	if ( a_uiDataLength == 0 )
		return;

	while ( m_bIsPlaying )
	{
		const THEORAPLAY_AudioPacket* pAudio       = m_pTheoraAudio;
		TINT                          iAudioOffset = m_iAudioOffset;

		if ( pAudio == TNULL &&
		     ( THEORAPLAY_isDecoding( m_pTheoraDecoder ) == 0 || !m_bIsPlaying || ( pAudio = THEORAPLAY_getAudio( m_pTheoraDecoder ) ) == TNULL ) )
		{
			memset( pData, 0, a_uiDataLength );
			return;
		}

		TINT    iChannels   = pAudio->channels;
		TUINT32 uiSize      = TUINT32( ( pAudio->frames - iAudioOffset ) * iChannels );
		TUINT32 uiNumFloats = a_uiDataLength >> 2;

		if ( uiNumFloats <= uiSize && uiSize - uiNumFloats != 0 )
			uiSize = uiNumFloats;

		memcpy( pData, pAudio->samples + iChannels * iAudioOffset, uiSize * 4 );
		pData += uiSize * 4;

		TINT iNewAudioOffset = iAudioOffset + TINT( uiSize ) / iChannels;
		a_uiDataLength -= uiSize * 4;

		if ( TUINT32( pAudio->frames ) <= TUINT32( iNewAudioOffset ) )
		{
			THEORAPLAY_freeAudio( pAudio );
			pAudio          = TNULL;
			iNewAudioOffset = 0;
		}

		m_pTheoraAudio = pAudio;
		m_iAudioOffset = iNewAudioOffset;

		if ( a_uiDataLength == 0 )
			return;
	}
}

// $deBlob: FUNCTION 00551be0
void ADX11MoviePlayer::CreateTextures( TUINT32 a_uiWidth, TUINT32 a_uiHeight )
{
	TRenderDX11* pRender = TRenderDX11::Interface();

	for ( TINT i = 0; i < 2; i++ )
	{
		m_apTextures[ i ]     = pRender->CreateTexture( a_uiWidth, a_uiHeight, DXGI_FORMAT_A8_UNORM, TNULL, 0, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 1 );
		m_apTextures[ i + 2 ] = pRender->CreateTexture( a_uiWidth >> 1, a_uiHeight >> 1, DXGI_FORMAT_A8_UNORM, TNULL, 0, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 1 );
		m_apTextures[ i + 4 ] = pRender->CreateTexture( a_uiWidth >> 1, a_uiHeight >> 1, DXGI_FORMAT_A8_UNORM, TNULL, 0, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, 1 );
	}
}

// $deBlob: FUNCTION 00551c80
void ADX11MoviePlayer::ReleaseTextures()
{
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
}

// $deBlob: FUNCTION 00551ce0
void ADX11MoviePlayer::CompileShader()
{
	TRenderDX11* pRender = TRenderDX11::Interface();
	m_pVertexBuffer      = pRender->CreateBuffer( TRenderDX11::BUFFERTYPE_VERTEX, sizeof( s_afScreenMesh ), s_afScreenMesh, D3D11_USAGE_IMMUTABLE, 0 );

	ID3DBlob* pPSBlob = TRenderDX11::CompileShader( s_szPixelShader, "main", "ps_4_0_level_9_3", TNULL );
	pRender->CreatePixelShader( pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), &m_pPixelShader );
	pPSBlob->Release();

	ID3DBlob* pVSBlob = TRenderDX11::CompileShader( s_szVertexShader, "main", "vs_4_0_level_9_3", TNULL );
	pRender->CreateVertexShader( pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexShader );

	D3D11_INPUT_ELEMENT_DESC oInputDesc;
	oInputDesc.SemanticName         = "POSITION";
	oInputDesc.SemanticIndex        = 0;
	oInputDesc.Format               = DXGI_FORMAT_R32G32_FLOAT;
	oInputDesc.InputSlot            = 0;
	oInputDesc.AlignedByteOffset    = D3D11_APPEND_ALIGNED_ELEMENT;
	oInputDesc.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
	oInputDesc.InstanceDataStepRate = 0;

	pRender->GetDevice()->CreateInputLayout( &oInputDesc, 1, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pInputLayout );
	pVSBlob->Release();
}

// $deBlob: FUNCTION 00551de0
void ADX11MoviePlayer::ReleaseShaders()
{
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

	if ( m_pVertexBuffer != TNULL )
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = TNULL;
	}
}

// $deBlob: FUNCTION 00551e40
void ADX11MoviePlayer::StopMovieImpl()
{
	if ( m_bHasAudioStream && m_pChannel != TNULL )
	{
		FMOD::Sound* pSound = TNULL;
		m_bIsPlaying        = TFALSE;
		m_pChannel->getCurrentSound( &pSound );
		m_pChannel->stop();

		// Waits for the stream thread to stop reading (ReadBuffer)
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

	m_bIsPlaying = TFALSE;
	m_bIsPaused  = TFALSE;
	m_bIsHidden  = TTRUE;
}
