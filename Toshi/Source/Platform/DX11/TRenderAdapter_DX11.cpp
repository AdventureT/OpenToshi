#include "ToshiPCH.h"
#include "TRenderAdapter_DX11.h"
#include "TRender_DX11.h"
#include "Toshi/TStringManager.h"
#include "Toshi/TUtil.h"

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

const TCHAR* TD3DAdapter::Mode::Device::DEVICETYPES[ NUMSUPPORTEDDEVICES ] = { "HAL", "REF" };

//-----------------------------------------------------------------------------
// TD3DAdapter::Mode::Device
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006b32f0
TD3DAdapter::Mode::Device::Device()
{
	m_eFlags                  = 0;
	m_pOwnerMode              = TNULL;
	m_uiDeviceIndex           = 0;
	m_bIsSoftware             = TFALSE;
	m_bCanRenderWindowed      = TFALSE;
	m_bSupportsTransformation = TFALSE;
	m_bSupportsPureDevice     = TFALSE;
	m_bSupportsNPatches       = TFALSE;
	TUtil::MemClear( m_aSupportedDepthStencilFormats, sizeof( m_aSupportedDepthStencilFormats ) );
}

// $deBlob: FUNCTION 006b3340
TRenderAdapter::Mode* TD3DAdapter::Mode::Device::GetMode() const
{
	return m_pOwnerMode;
}

// $deBlob: FUNCTION 006b3350
TUINT32 TD3DAdapter::Mode::Device::GetDeviceIndex() const
{
	return m_uiDeviceIndex;
}

// $deBlob: FUNCTION 006b3360
const TCHAR* TD3DAdapter::Mode::Device::GetTypeString() const
{
	return DEVICETYPES[ m_uiDeviceIndex ];
}

// $deBlob: FUNCTION 006b3370
TBOOL TD3DAdapter::Mode::Device::IsSoftware() const
{
	return m_bIsSoftware;
}

// $deBlob: FUNCTION 006b3380
TBOOL TD3DAdapter::Mode::Device::CanRenderWindowed() const
{
	return m_bCanRenderWindowed;
}

// $deBlob: FUNCTION 006b3390
TBOOL TD3DAdapter::Mode::Device::SupportsHardwareTransfomations() const
{
	return m_bSupportsTransformation;
}

// $deBlob: FUNCTION 006b33a0
TBOOL TD3DAdapter::Mode::Device::SupportsPureDevice() const
{
	return m_bSupportsPureDevice;
}

// $deBlob: FUNCTION 006b33b0
TBOOL TD3DAdapter::Mode::Device::IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const
{
	TASSERT( ( TINT( a_iIndex ) >= 0 ) && ( TINT( a_iIndex ) < TRender::DEPTHSTENCILFORMAT_NUMOF ) );
	return m_aSupportedDepthStencilFormats[ a_iIndex ];
}

//-----------------------------------------------------------------------------
// TD3DAdapter::Mode
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006a9d70
TD3DAdapter::Mode::Mode()
{
	// NOTE: retail leaves the other members uninitialised
}

// $deBlob: FUNCTION 006aa0f0
// $deBlob: FUNCTION 006aa710
// $deBlob: FUNCTION 006aa740
TD3DAdapter::Mode::~Mode()
{
}

// $deBlob: FUNCTION 006b3450
TRenderAdapter* TD3DAdapter::Mode::GetAdapter() const
{
	return m_pOwnerAdapter;
}

// $deBlob: FUNCTION 006b3460
size_t TD3DAdapter::Mode::GetModeIndex() const
{
	return m_uiModeIndex;
}

// $deBlob: FUNCTION 006b3470
TUINT32 TD3DAdapter::Mode::GetWidth() const
{
	return m_Description.Width;
}

// $deBlob: FUNCTION 006b3480
TUINT32 TD3DAdapter::Mode::GetHeight() const
{
	return m_Description.Height;
}

// $deBlob: FUNCTION 006b3490
TBOOL TD3DAdapter::Mode::Is32Bit() const
{
	return m_Description.Format == DXGI_FORMAT_B8G8R8X8_UNORM || m_Description.Format == DXGI_FORMAT_R8G8B8A8_UNORM;
}

// $deBlob: FUNCTION 006b34b0
TBOOL TD3DAdapter::Mode::Is16Bit() const
{
	return m_Description.Format == DXGI_FORMAT_B5G6R5_UNORM || m_Description.Format == DXGI_FORMAT_B5G5R5A1_UNORM || m_Description.Format == DXGI_FORMAT_B4G4R4A4_UNORM;
}

// $deBlob: FUNCTION 006b34d0
TUINT32 TD3DAdapter::Mode::GetRefreshRate() const
{
	// Rounded (fistp) to an integer, then converted back and forth
	const TFLOAT fRefreshRate = TFLOAT( m_Description.RefreshRate.Numerator ) / TFLOAT( m_Description.RefreshRate.Denominator );
	return TUINT32( TFLOAT( TINT( lrintf( fRefreshRate ) ) ) );
}

// $deBlob: FUNCTION 006b3530
TRenderAdapter::Mode::Device* TD3DAdapter::Mode::GetDevice( TINT a_iDevice )
{
	TASSERT( ( a_iDevice >= 0 ) && ( a_iDevice < NUMSUPPORTEDDEVICES ) );

	if ( TUINT32( a_iDevice ) < NUMSUPPORTEDDEVICES )
		return &m_aDevices[ a_iDevice ];

	return TNULL;
}

// $deBlob: FUNCTION 006b35a0
void TD3DAdapter::Mode::SetDescription( const DXGI_MODE_DESC& a_rDescription )
{
	m_Description = a_rDescription;
}

// $deBlob: FUNCTION 006b35e0
DXGI_MODE_DESC* TD3DAdapter::Mode::GetDescription()
{
	return &m_Description;
}

// $deBlob: FUNCTION 006b39e0
void TD3DAdapter::Mode::GetDisplayMode( IDXGIOutput* a_pOutput, DXGI_MODE_DESC* a_pModeDesc )
{
	DXGI_OUTPUT_DESC oOutputDesc;
	a_pOutput->GetDesc( &oOutputDesc );

	MONITORINFOEXA oMonitorInfo;
	oMonitorInfo.cbSize = sizeof( oMonitorInfo );
	GetMonitorInfoA( oOutputDesc.Monitor, &oMonitorInfo );

	DEVMODEA oDisplaySettings;
	oDisplaySettings.dmSize        = sizeof( oDisplaySettings );
	oDisplaySettings.dmDriverExtra = 0;
	EnumDisplaySettingsA( oMonitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &oDisplaySettings );

	// 0 and 1 mean the hardware's default refresh rate
	const TBOOL bDefaultRefreshRate = oDisplaySettings.dmDisplayFrequency == 1 || oDisplaySettings.dmDisplayFrequency == 0;

	DXGI_MODE_DESC oMatchMode;
	oMatchMode.Width                   = oDisplaySettings.dmPelsWidth;
	oMatchMode.Height                  = oDisplaySettings.dmPelsHeight;
	oMatchMode.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
	oMatchMode.ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	oMatchMode.Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
	oMatchMode.RefreshRate.Numerator   = bDefaultRefreshRate ? 0 : oDisplaySettings.dmDisplayFrequency;
	oMatchMode.RefreshRate.Denominator = !bDefaultRefreshRate;

	a_pOutput->FindClosestMatchingMode( &oMatchMode, a_pModeDesc, NULL );
}

//-----------------------------------------------------------------------------
// TD3DAdapter
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006a9e10
TD3DAdapter::TD3DAdapter()
{
	// NOTE: retail leaves the other members uninitialised (m_szDriver is never set)
#ifdef DEBLOB_COMMUNITY_PATCH
	m_szDriver[ 0 ] = '\0';
#endif
}

// $deBlob: FUNCTION 006b3600
// $deBlob: FUNCTION 006aa7a0
TD3DAdapter::~TD3DAdapter()
{
}

// $deBlob: FUNCTION 006b3620
TUINT32 TD3DAdapter::GetAdapterIndex() const
{
	return m_uiAdapterIndex;
}

// $deBlob: FUNCTION 006b3630
const TCHAR* TD3DAdapter::GetDriver() const
{
	return m_szDriver;
}

// $deBlob: FUNCTION 006b3640
const TCHAR* TD3DAdapter::GetDriverDescription() const
{
	return m_szDescription;
}

// NOTE: the four version getters read the LUID of the adapter (TOSHI's DX8 adapter kept the driver version there)

// $deBlob: FUNCTION 006b3650
TUINT TD3DAdapter::GetProductID() const
{
	return HIWORD( m_AdapterLuid.HighPart );
}

// $deBlob: FUNCTION 006b3660
TUINT TD3DAdapter::GetVersion() const
{
	return LOWORD( m_AdapterLuid.HighPart );
}

// $deBlob: FUNCTION 006b3670
TUINT TD3DAdapter::GetSubVersion() const
{
	return HIWORD( m_AdapterLuid.LowPart );
}

// $deBlob: FUNCTION 006b3680
TUINT TD3DAdapter::GetBuild() const
{
	return LOWORD( m_AdapterLuid.LowPart );
}

// $deBlob: FUNCTION 006b3690
TUINT32 TD3DAdapter::GetNumSupportedDevices() const
{
	return Mode::NUMSUPPORTEDDEVICES;
}

// $deBlob: FUNCTION 006b36b0
// (called after EnumerateOutputs, only by BuildAdapterDatabase); empty in de Blob
void TD3DAdapter::EnumerateDevices( TRenderDX11* a_pRender )
{
}

// $deBlob: FUNCTION 006b36d0
void TD3DAdapter::EnumerateOutputs( TRenderDX11* a_pRender, IDXGIAdapter* a_pDXGIAdapter )
{
	GetModeList()->DeleteAll();

	TBOOL        bFirstOutput   = TTRUE;
	IDXGIOutput* pOutput        = TNULL;
	TUINT32      uiDisplayIndex = 0;

	// NOTE: retail never releases the outputs
	while ( a_pDXGIAdapter->EnumOutputs( uiDisplayIndex, &pOutput ) != DXGI_ERROR_NOT_FOUND )
	{
		DXGI_OUTPUT_DESC oOutputDesc;
		pOutput->GetDesc( &oOutputDesc );

		TCHAR szDeviceName[ 128 ];
		TUtil::Log(
		    "Display[%d]: %s (%dx%d)",
		    uiDisplayIndex,
		    TStringManager::StringUnicodeToChar( szDeviceName, oOutputDesc.DeviceName, -1 ),
		    oOutputDesc.DesktopCoordinates.right - oOutputDesc.DesktopCoordinates.left,
		    oOutputDesc.DesktopCoordinates.bottom - oOutputDesc.DesktopCoordinates.top
		);

		UINT uiNumModes = 0;
		pOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uiNumModes, NULL );

		if ( uiNumModes != 0 )
		{
			DXGI_MODE_DESC oDesktopMode;
			Mode::GetDisplayMode( pOutput, &oDesktopMode );

			// NOTE: retail copies 0x5C bytes of the 0x40 bytes of DeviceName (the whole DXGI_OUTPUT_DESC)
			TSTATICASSERT( sizeof( m_Mode.m_wszDisplayName ) == 0x5C );

			if ( bFirstOutput )
			{
				m_Mode.m_pOwnerAdapter = this;
				m_Mode.m_Description   = oDesktopMode;
				bFirstOutput           = TFALSE;
				TUtil::MemCopy( m_Mode.m_wszDisplayName, oOutputDesc.DeviceName, sizeof( m_Mode.m_wszDisplayName ) );
			}

			DXGI_MODE_DESC* pModes = TSTATICCAST( DXGI_MODE_DESC, TMemalign( 16, uiNumModes * sizeof( DXGI_MODE_DESC ) ) );
			pOutput->GetDisplayModeList( DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &uiNumModes, pModes );

			for ( UINT i = 0; i < uiNumModes; i++ )
			{
				Mode* pMode            = new Mode();
				pMode->m_uiModeIndex   = i;
				pMode->m_pOwnerAdapter = this;
				pMode->m_Description   = pModes[ i ];
				pMode->m_uiDisplayIndex = uiDisplayIndex;
				TUtil::MemCopy( pMode->m_wszDisplayName, oOutputDesc.DeviceName, sizeof( pMode->m_wszDisplayName ) );

				GetModeList()->InsertTail( pMode );
			}

			operator delete( pModes );
		}

		uiDisplayIndex++;
	}
}

TOSHI_NAMESPACE_END
