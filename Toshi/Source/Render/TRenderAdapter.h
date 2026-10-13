#pragma once
#include "Toshi/TNodeList.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Adapter database of the renderer (TRender::GetAdapterList). de Blob's platform classes are TD3DAdapter,
// TD3DAdapter::Mode and TD3DAdapter::Mode::Device (Source\Render\TRenderAdapter_DX11.cpp).
//
// NOTE: compared to OpenBarnyard, de Blob's adapter has no GetDeviceIdentifier (GUID) and its driver strings are
// character arrays, its device has a ninth virtual (SupportsNPatches?).
//-----------------------------------------------------------------------------
class TRenderAdapter : public TNodeList<TRenderAdapter>::TNode
{
public:
	class Mode : public TNodeList<Mode>::TNode
	{
	public:
		class Device : public TNodeList<Device>::TNode
		{
		public:
			Device() = default;
			virtual ~Device() = default;                                                // +0x00

			virtual Mode*        GetMode() const                                         = 0; // +0x04
			virtual TUINT32      GetDeviceIndex() const                                  = 0; // +0x08
			virtual const TCHAR* GetTypeString() const                                   = 0; // +0x0C
			virtual TBOOL        IsSoftware() const                                      = 0; // +0x10
			virtual TBOOL        CanRenderWindowed() const                               = 0; // +0x14
			virtual TBOOL        SupportsHardwareTransfomations() const                  = 0; // +0x18
			virtual TBOOL        IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const = 0; // +0x1C
			virtual TBOOL        SupportsPureDevice() const                              = 0; // +0x20
		};

	public:
		Mode() = default;
		virtual ~Mode() = default; // +0x00

		virtual TRenderAdapter*               GetAdapter() const            = 0; // +0x04
		virtual size_t                        GetModeIndex() const          = 0; // +0x08
		virtual TUINT32                       GetWidth() const              = 0; // +0x0C
		virtual TUINT32                       GetHeight() const             = 0; // +0x10
		virtual TBOOL                         Is32Bit() const               = 0; // +0x14
		virtual TBOOL                         Is16Bit() const               = 0; // +0x18
		virtual TUINT32                       GetRefreshRate() const        = 0; // +0x1C
		virtual TRenderAdapter::Mode::Device* GetDevice( TINT a_iDevice )   = 0; // +0x20
	};

public:
	TRenderAdapter() = default;
	// NOTE: deletes the modes (the list at +0x10)
	// $deBlob: FUNCTION 006aa240
	// $deBlob: FUNCTION 006aa8f0
	virtual ~TRenderAdapter() { m_Modes.DeleteAll(); } // +0x00

	virtual TUINT32      GetAdapterIndex() const        = 0; // +0x04
	virtual const TCHAR* GetDriver() const              = 0; // +0x08
	virtual const TCHAR* GetDriverDescription() const   = 0; // +0x0C
	virtual TUINT16      GetProductID() const           = 0; // +0x10
	virtual TUINT16      GetVersion() const             = 0; // +0x14
	virtual TUINT16      GetSubVersion() const          = 0; // +0x18
	virtual TUINT16      GetBuild() const               = 0; // +0x1C
	virtual TUINT32      GetNumSupportedDevices() const = 0; // +0x20

	TNodeList<Mode>* GetModeList() { return &m_Modes; }

private:
	TNodeList<Mode> m_Modes; // 0x10
};

TOSHI_NAMESPACE_END
