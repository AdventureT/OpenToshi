#pragma once
#include "Render/TRenderAdapter.h"

#include <d3d11.h>
#include <dxgi.h>

TOSHI_NAMESPACE_START

class TRenderDX11;

//-----------------------------------------------------------------------------
// de Blob: Source\Render\TRenderAdapter_DX11.cpp. One adapter per DXGI adapter, one mode per display mode of
// each of its outputs (TRenderDX11::BuildAdapterDatabase 006a7df0).
//
// NOTE: de Blob never fills the devices of the modes (they stay as constructed: no depth/stencil format and
// no windowed support), so TRender::FindDevice never finds one. TRenderDX11::GetCurrentDevice returns TNULL.
//-----------------------------------------------------------------------------
class TD3DAdapter : public TRenderAdapter
{
public:
	class Mode : public TRenderAdapter::Mode
	{
	public:
		static constexpr TINT NUMSUPPORTEDDEVICES = 2;
		static constexpr TINT DISPLAYNAMESIZE     = 46;

		class Device : public TRenderAdapter::Mode::Device
		{
		public:
			static constexpr const TCHAR* DEVICETYPES[ NUMSUPPORTEDDEVICES ] = { "HAL", "REF" }; // 0x00981fdc

		public:
			Device();
			~Device() = default;

			//-----------------------------------------------------------------------------
			// TRenderAdapter::Mode::Device
			//-----------------------------------------------------------------------------
			virtual TRenderAdapter::Mode* GetMode() const OVERRIDE;
			virtual TUINT32               GetDeviceIndex() const OVERRIDE;
			virtual const TCHAR*          GetTypeString() const OVERRIDE;
			virtual TBOOL                 IsSoftware() const OVERRIDE;
			virtual TBOOL                 CanRenderWindowed() const OVERRIDE;
			virtual TBOOL                 SupportsHardwareTransfomations() const OVERRIDE;
			virtual TBOOL                 IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const OVERRIDE;
			virtual TBOOL                 SupportsPureDevice() const OVERRIDE;

		private:
			TUINT32 m_eFlags;                       // 0x10
			Mode*   m_pOwnerMode;                   // 0x14
			TUINT32 m_uiDeviceIndex;                // 0x18
			TBOOL   m_bIsSoftware;                  // 0x1C
			TBOOL   m_bCanRenderWindowed;           // 0x1D
			TBOOL   m_bSupportsTransformation;      // 0x1E
			TBOOL   m_bSupportsPureDevice;          // 0x1F
			TBOOL   m_bSupportsNPatches;            // 0x20
			TBOOL   m_aSupportedDepthStencilFormats[ 5 ]; // 0x21
		};

	public:
		Mode();
		~Mode();

		//-----------------------------------------------------------------------------
		// TRenderAdapter::Mode
		//-----------------------------------------------------------------------------
		virtual TRenderAdapter*               GetAdapter() const OVERRIDE;
		virtual size_t                        GetModeIndex() const OVERRIDE;
		virtual TUINT32                       GetWidth() const OVERRIDE;
		virtual TUINT32                       GetHeight() const OVERRIDE;
		virtual TBOOL                         Is32Bit() const OVERRIDE;
		virtual TBOOL                         Is16Bit() const OVERRIDE;
		virtual TUINT32                       GetRefreshRate() const OVERRIDE;
		virtual TRenderAdapter::Mode::Device* GetDevice( TINT a_iDevice ) OVERRIDE;

		void SetDescription( const DXGI_MODE_DESC& a_rDescription );

		// $deBlob: FUNCTION 006b35e0
		DXGI_MODE_DESC*       GetDescription() { return &m_Description; }
		const WCHAR*          GetDisplayName() const { return m_wszDisplayName; }
		TUINT32               GetDisplayIndex() const { return m_uiDisplayIndex; }

		// The desktop mode of an output (EnumDisplaySettings of its monitor, matched by the output)
		static void GetDisplayMode( IDXGIOutput* a_pOutput, DXGI_MODE_DESC* a_pModeDesc );

	private:
		friend TD3DAdapter;

		TD3DAdapter*   m_pOwnerAdapter;                      // 0x10
		DXGI_MODE_DESC m_Description;                        // 0x14
		TUINT32        m_uiModeIndex;                        // 0x30
		Device         m_aDevices[ NUMSUPPORTEDDEVICES ];    // 0x34
		TUINT32        m_uiDisplayIndex;                     // 0x84
		WCHAR          m_wszDisplayName[ DISPLAYNAMESIZE ];  // 0x88 DXGI_OUTPUT_DESC::DeviceName
	};

public:
	TD3DAdapter();
	~TD3DAdapter();

	//-----------------------------------------------------------------------------
	// TRenderAdapter
	//-----------------------------------------------------------------------------
	virtual TUINT32      GetAdapterIndex() const OVERRIDE;
	virtual const TCHAR* GetDriver() const OVERRIDE;
	virtual const TCHAR* GetDriverDescription() const OVERRIDE;
	virtual TUINT16      GetProductID() const OVERRIDE;
	virtual TUINT16      GetVersion() const OVERRIDE;
	virtual TUINT16      GetSubVersion() const OVERRIDE;
	virtual TUINT16      GetBuild() const OVERRIDE;
	virtual TUINT32      GetNumSupportedDevices() const OVERRIDE;

	// Adds the display modes of every output of the adapter, the desktop mode of the first output is GetMode()
	void EnumerateOutputs( TRenderDX11* a_pRender, IDXGIAdapter* a_pDXGIAdapter );

	// $deBlob: FUNCTION 006b39d0
	// The desktop mode of the first output
	Mode* GetMode() { return &m_Mode; }

	const DXGI_ADAPTER_DESC& GetAdapterDesc() const { return m_AdapterDesc; }

private:
	friend TRenderDX11;

	DXGI_ADAPTER_DESC m_AdapterDesc;                // 0x20
	Mode              m_Mode;                       // 0x144
	TUINT32           m_uiAdapterIndex;             // 0x228
	TCHAR             m_szDriver[ 512 ];            // 0x22C never set (uninitialised)
	TCHAR             m_szDescription[ 512 ];       // 0x42C
	LUID              m_AdapterLuid;                // 0x62C read by GetBuild/GetSubVersion/GetVersion/GetProductID
	UINT              m_VendorId;                   // 0x634
	UINT              m_DeviceId;                   // 0x638
	UINT              m_SubSysId;                   // 0x63C
	UINT              m_Revision;                   // 0x640
};

TOSHI_NAMESPACE_END
