#include "pch.h"
#include "ARenderer.h"
#include "AApplication.h"

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TRender_DX11.h>
#  include <Platform/DX11/TRenderAdapter_DX11.h>
#endif

#include <Render/TNullResource.h>
#include <Render/TVertexFactoryResourceInterface.h>
#include <Render/TIndexFactoryResourceInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005f0780
TBOOL ARenderer::CreateTRender()
{
	TRenderDX11* pRender = new TRenderDX11();
	pRender->SetAutoCreateSystemResources( TFALSE );
	pRender->Create();

	m_DisplayModes.Initialise();

	TINT    iWidth        = g_oTheApp.m_iWidth;
	TINT    iHeight       = g_oTheApp.m_iHeight;
	TUINT32 uiColourDepth = 32;
	TBOOL   bWindowed     = g_oTheApp.m_bWindowed;
	TBOOL   bFullscreen   = g_oTheApp.m_bExclusiveFullscreen;

#ifdef DEBLOB_COMMUNITY_PATCH
	// "-background" runs (see TMSWindow::IsBackgroundMode) are always windowed
	if ( TMSWindow::IsBackgroundMode() )
	{
		bWindowed   = TTRUE;
		bFullscreen = TFALSE;
	}
#endif

	TD3DAdapter::Mode* pMode = TSTATICCAST( TD3DAdapter, pRender->GetAdapterList()->Head().Get() )->GetMode();

	const TCHAR* szText;

	if ( iWidth == -1 )
	{
		if ( iHeight != -1 )
		{
			szText = "You should specify both width and heigth or neither";
			MessageBoxA( NULL, szText, "Invalid args", MB_ICONHAND | MB_SYSTEMMODAL );
			return TFALSE;
		}

		// Neither: the default size (windowed) or the desktop's
		iWidth  = s_uiDefaultWidth;
		iHeight = s_uiDefaultHeight;

		if ( !bWindowed )
		{
			iWidth  = pMode->GetWidth();
			iHeight = pMode->GetHeight();
		}
	}
	else
	{
		if ( iHeight == -1 )
		{
			szText = "You should specify both width and heigth or neither";
			MessageBoxA( NULL, szText, "Invalid args", MB_ICONHAND | MB_SYSTEMMODAL );
			return TFALSE;
		}

		if ( !bWindowed )
		{
			TUINT32 uiWidth  = iWidth;
			TUINT32 uiHeight = iHeight;

			if ( iWidth < 1280 || iHeight < 720 || !m_DisplayModes.DoesModeExist( uiWidth, uiHeight, uiColourDepth ) )
			{
				szText = "The specified width and heith must be greater than 1280x720 and match a valid mode";
				MessageBoxA( NULL, szText, "Invalid args", MB_ICONHAND | MB_SYSTEMMODAL );
				return TFALSE;
			}
		}
		else if ( iWidth < 1280 || TUINT32( iWidth ) > pMode->GetWidth() || iHeight < 720 || TUINT32( iHeight ) > pMode->GetHeight() )
		{
			TCHAR szBuffer[ 512 ];
			T2String8::Format( szBuffer, "The specified width and heith must be between 1280x720 and the current desktop resolution (%dx%d)", pMode->GetWidth(), pMode->GetHeight() );
			MessageBoxA( NULL, szBuffer, "Invalid args", MB_ICONHAND | MB_SYSTEMMODAL );
			return TFALSE;
		}
	}

	TRender::DISPLAYPARAMS oDisplayParams;
	oDisplayParams.uiWidth                   = iWidth;
	oDisplayParams.uiHeight                  = iHeight;
	oDisplayParams.uiColourDepth             = uiColourDepth;
	oDisplayParams.eDepthStencilFormat       = TRender::DEPTHSTENCILFORMAT( 3 );
	oDisplayParams.bWindowed                 = bWindowed;
	oDisplayParams.bFullscreen               = bFullscreen;
	oDisplayParams.uiMultisampleQualityLevel = 1;

	return pRender->CreateDisplay( oDisplayParams ) == TTRUE;
}

// $deBlob: FUNCTION 005f0a50
void ARenderer::CreateTRenderResources()
{
	TRender* pRender = TRender::GetSingleton();

	pRender->SetResourceExplicit( pRender->CreateResource( &TGetClass( TNullResource ), "VFactories", TNULL ), TRender::SYSRESOURCE_VFactories );

	// The vertices of TSysShader: 0x18 bytes
	{
		TVertexFactoryFormat oFormat;
		pRender->SetResourceExplicit( pRender->CreateResource( TClass::Find( "TVertexFactoryResource" ), "VFSYSVNDUV1", pRender->GetSystemResource( TRender::SYSRESOURCE_VFactories ) ), TRender::SYSRESOURCE_VFSYSVNDUV1 );

		oFormat.m_uiNumStreams                     = 1;
		oFormat.m_aStreamFormats[ 0 ].m_uiVertexSize = 0x18;
		oFormat.m_aStreamFormats[ 0 ].m_Unk          = 0;

		TBOOL bRes = pRender->GetSystemResource<TVertexFactoryResourceInterface>( TRender::SYSRESOURCE_VFSYSVNDUV1 )->Create( &oFormat, 0xD322, 0 );
		TASSERT( TTRUE == bRes );
	}

	// The skinned vertices: 0x2C bytes
	{
		TVertexFactoryFormat oFormat;
		pRender->SetResourceExplicit( pRender->CreateResource( TClass::Find( "TVertexFactoryResource" ), "VFSKIN", pRender->GetSystemResource( TRender::SYSRESOURCE_VFactories ) ), TRender::SYSRESOURCE_VFSKIN );

		oFormat.m_uiNumStreams                     = 1;
		oFormat.m_aStreamFormats[ 0 ].m_uiVertexSize = 0x2C;
		oFormat.m_aStreamFormats[ 0 ].m_Unk          = 0;

		TBOOL bRes = pRender->GetSystemResource<TVertexFactoryResourceInterface>( TRender::SYSRESOURCE_VFSKIN )->Create( &oFormat, 0xD322, 0 );
		TASSERT( TTRUE == bRes );
	}

	// The world vertices: 0x34 bytes
	{
		TVertexFactoryFormat oFormat;
		pRender->SetResourceExplicit( pRender->CreateResource( TClass::Find( "TVertexFactoryResource" ), "VFWORLD", pRender->GetSystemResource( TRender::SYSRESOURCE_VFactories ) ), TRender::SYSRESOURCE_VFWORLD );

		oFormat.m_uiNumStreams                     = 1;
		oFormat.m_aStreamFormats[ 0 ].m_uiVertexSize = 0x34;
		oFormat.m_aStreamFormats[ 0 ].m_Unk          = 0;

		TBOOL bRes = pRender->GetSystemResource<TVertexFactoryResourceInterface>( TRender::SYSRESOURCE_VFWORLD )->Create( &oFormat, 0xD322, 0 );
		TASSERT( TTRUE == bRes );
	}

	// The 16-bit indices
	pRender->SetResourceExplicit( pRender->CreateResource( &TGetClass( TNullResource ), "IFactories", TNULL ), TRender::SYSRESOURCE_IFactories );
	pRender->SetResourceExplicit( pRender->CreateResource( TClass::Find( "TIndexFactoryResource" ), "IFSYS", pRender->GetSystemResource( TRender::SYSRESOURCE_IFactories ) ), TRender::SYSRESOURCE_IFSYS );

	TIndexFactoryFormat oIndexFormat;
	oIndexFormat.uiIndexSize = 2;

	TBOOL bRes = pRender->GetSystemResource<TIndexFactoryResourceInterface>( TRender::SYSRESOURCE_IFSYS )->Create( &oIndexFormat, 0xFCBC, 0 );
	TASSERT( TTRUE == bRes );
}
