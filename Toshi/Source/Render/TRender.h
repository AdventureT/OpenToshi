/**
 * @file TRender.h
 * Abstract renderer of the Toshi engine (de Blob: Source\Render\TRender.cpp, class TRender)
 */

#pragma once
#include "Toshi/Defines.h"
#include "Math/Math.h"
#include "Math/TSphere.h"
#include "Toshi/TStack.h"
#include "Toshi/TNodeList.h"
#include "Toshi/TRefCounted.h"
#include "Render/TResource.h"
#include "Render/TAnimation.h"
#include "Render/TRenderAdapter.h"
#include "Render/TRenderContext.h"
#include "Render/TModel.h"

TOSHI_NAMESPACE_START

class TRenderParamTable;

//-----------------------------------------------------------------------------
// de Blob's abstract renderer (vftable 008c80d8, 19 slots, size 0x61C). The platform renderer is TRenderDX11.
//
// NOTE: OpenBarnyard's TRenderInterface is the same class of a later TOSHI 2.0 branch; de Blob's has fewer virtuals
// (no capture/debug text/model factories/pixel aspect ratio), only 12 system resources, an aspect ratio, a shader
// parameter table and no begin/end scene emitters.
//-----------------------------------------------------------------------------
class TRender
    : public TObject
    , public TSingleton<TRender>
    , public TRefCounted
{
public:
	TDECLARE_CLASS( TRender, TObject );

public:
	static constexpr size_t MAXNUMRESOURCES = 6000;

	// Indices of the system resources (set by ARenderer::CreateTRender's resources 005f0a50)
	typedef TUINT8 SYSRESOURCE;
	enum SYSRESOURCE_ : SYSRESOURCE
	{
		SYSRESOURCE_VFactories,
		SYSRESOURCE_VFSYSVNDUV1,
		SYSRESOURCE_VFSKIN,
		SYSRESOURCE_3,
		SYSRESOURCE_4,
		SYSRESOURCE_5,
		SYSRESOURCE_VFWORLD,
		SYSRESOURCE_7,
		SYSRESOURCE_8,
		SYSRESOURCE_IFactories,
		SYSRESOURCE_IFSYS,
		SYSRESOURCE_11,
		SYSRESOURCE_NUMOF
	};

	typedef TUINT32 ASPECT_RATIO;
	enum ASPECT_RATIO_ : ASPECT_RATIO
	{
		ASPECT_RATIO_4_3,
		ASPECT_RATIO_16_9,
	};

	// Index of the format in TRenderAdapter::Mode::Device::IsDepthStencilFormatSupported (see GetDepthStencilFormat)
	using DEPTHSTENCILFORMAT = TUINT32;
	enum DEPTHSTENCILFORMAT_ : DEPTHSTENCILFORMAT
	{
		DEPTHSTENCILFORMAT_D16    = 0,
		DEPTHSTENCILFORMAT_D15_S1 = 1,
		DEPTHSTENCILFORMAT_D24_X8 = 2,
		DEPTHSTENCILFORMAT_D24_S8 = 3,
		DEPTHSTENCILFORMAT_D32    = 4,
		DEPTHSTENCILFORMAT_NUMOF  = 5,
	};

	// TRender_DisplayParams (see ARenderer::CreateTRender 005f0780)
	struct DISPLAYPARAMS
	{
		TUINT32            uiWidth;                   // 0x00
		TUINT32            uiHeight;                  // 0x04
		TUINT32            uiColourDepth;             // 0x08
		DEPTHSTENCILFORMAT eDepthStencilFormat;       // 0x0C
		TBOOL              bWindowed;                 // 0x10 "-windowed" mode, neither flag is a borderless window of the desktop's size
		TBOOL              bFullscreen;               // 0x11 exclusive fullscreen ("-exclusivefullscreen")
		TUINT32            uiMultisampleQualityLevel; // 0x14 sample count (1, 2, 4 or 8)
	};

public:
	TRender();
	virtual ~TRender();                                                             // +0x08

	virtual TBOOL                         CreateDisplay( const DISPLAYPARAMS& a_rParams ) = 0; // +0x0C
	virtual TBOOL                         DestroyDisplay()                                = 0; // +0x10
	virtual void                          Update( TFLOAT a_fDeltaTime );                       // +0x14
	virtual void                          BeginScene();                                        // +0x18
	virtual void                          EndScene();                                          // +0x1C
	virtual TRenderAdapter::Mode::Device* GetCurrentDevice()        = 0;                       // +0x20
	virtual DISPLAYPARAMS*                GetCurrentDisplayParams() = 0;                       // +0x24
	virtual TBOOL                         Create();                                            // +0x28
	virtual TBOOL                         Destroy();                                           // +0x2C
	virtual void                          DumpStats();                                         // +0x30
	virtual void                          GetScreenOffset( TVector2& a_rVec );                 // +0x34
	virtual void                          SetScreenOffset( const TVector2& a_rVec );           // +0x38
	virtual void                          SetLightDirectionMatrix( const TMatrix44& a_rMatrix ); // +0x3C
	virtual void                          SetLightColourMatrix( const TMatrix44& a_rMatrix );  // +0x40
	virtual TBOOL                         CreateSystemResources();                             // +0x44
	virtual void                          DestroySystemResources();                            // +0x48

	// Creates display and returns TTRUE if success
	TBOOL CreateDisplay();

	// Platform specific (TRender_DX11.cpp): creates the render context of the renderer
	TRenderContext* CreateRenderContext();

	// Flushes the order tables of the platform renderer (TRenderDX11::FlushOrderTables)
	void FlushShaders();

	// Destroys resource
	void DestroyResource( TResource* a_pResource );

	// Destroys resource recursively
	void DestroyResourceRecurse( TResource* a_pResource );

	// Destroys every resource of a_pClass (or derived) in the tree that starts at a_pResource, including their children.
	void DestroyResourcesOfClassRecurse( const TClass* a_pClass, TResource* a_pResource );

	// $deBlob: FUNCTION 00691110
	// Returns system resource
	template <class T = TResource>
	T* GetSystemResource( SYSRESOURCE a_SystemResource )
	{
		TASSERT( a_SystemResource < TRender::SYSRESOURCE_NUMOF );
		return TSTATICCAST( T, m_aSysResources[ a_SystemResource ] );
	}

	// Sets resource explicit
	void SetResourceExplicit( TResource* a_pResource, SYSRESOURCE a_eResource );

	// Creates resource and returns it
	TResource* CreateResource( const TClass* a_pClass, const TCHAR* a_szName, TResource* a_pParent );

	// Creates resource in a_pMemory (external, not deleted with the tree) and returns it
	TResource* CreateResourceInPlace( const TClass* a_pClass, void* a_pMemory, const TCHAR* a_szName, TResource* a_pParent );

	// $deBlob: FUNCTION 00691180
	// Sets new render context and returns the old one
	TRenderContext* SetCurrentRenderContext( TRenderContext* a_pRenderContext )
	{
		return std::exchange( m_pCurrentContext, a_pRenderContext );
	}

	// Height of the display relative to the height of the aspect ratio for its width
	TFLOAT GetResolutionScalar();

	void SetAutoCreateSystemResources( TBOOL a_bAutoCreate ) { m_bCreateSystemResources = a_bAutoCreate; }

	// $deBlob: FUNCTION 00690c20
	TBOOL IsInScene() { return m_bInScene; }
	// $deBlob: FUNCTION 00690c30
	TBOOL IsCreated() { return m_bCreated; }
	// $deBlob: FUNCTION 00690c40
	TBOOL IsDisplayCreated() { return m_bDisplayCreated; }

	const TMatrix44& GetLightDirection() const { return m_LightDirection; }
	const TMatrix44& GetLightColour() const { return m_LightColour; }

	// $deBlob: FUNCTION 00691170
	TRenderContext* GetCurrentContext() const { return m_pCurrentContext; }
	TRenderContext* GetCreatedContext() const { return m_pCreatedContext; }
	TUINT32         GetFrameCount() const { return m_iFrameCount; }
	// $deBlob: FUNCTION 00690590
	TNodeList<TRenderAdapter>* GetAdapterList() { return &m_AdapterList; }
	// $deBlob: FUNCTION 00690c90
	TNodeTree<TResource>*    GetResources() { return &m_Resources; }
	TKeyframeLibraryManager& GetKeyframeLibraryManager() { return m_KeyframeManager; }
	TStack<TMatrix44, 20>&   GetTransforms() { return m_Transforms; }
	TRenderParamTable*       GetParamTable() const { return m_pParamTable; }
	ASPECT_RATIO             GetAspectRatio() const { return m_eAspectRatio; }

	void FlushDyingResources();
	void DestroyDyingResources();
	void DestroyDyingResources( TResource* a_pResources );

	void DeleteResource( TResource* a_pResource );
	void DeleteResourceRecurse( TResource* a_pResource );
	void DeleteResourceAtomic( TResource* a_pResource );

	TRenderAdapter::Mode::Device* FindDevice( const DISPLAYPARAMS& a_rDisplayParams );

	// Index of the depth/stencil format with these bit counts (DEPTHSTENCILFORMAT) or -1
	static TINT GetDepthStencilFormat( TUINT32 a_uiDepthBits, TUINT32 a_uiStencilBits );

protected:
	TBOOL                     m_bCreateSystemResources;              // 0x08
	TBOOL                     m_bInScene;                            // 0x09
	TBOOL                     m_bCreated;                            // 0x0A
	TBOOL                     m_bDisplayCreated;                     // 0x0B
	TVector2                  m_ScreenOffset;                        // 0x0C
	ASPECT_RATIO              m_eAspectRatio;                        // 0x14
	TRenderContext*           m_pCurrentContext;                     // 0x18
	TRenderContext*           m_pCreatedContext;                     // 0x1C
	TResource*                m_aSysResources[ SYSRESOURCE_NUMOF ];  // 0x20
	TMatrix44                 m_LightDirection;                      // 0x50
	TMatrix44                 m_LightColour;                         // 0x90
	TObject*                  m_pUnk0D0;                             // 0xD0 deleted (TObject::Delete, vftable+4) by DestroyDisplay, never set in the code we know
	TRenderParamTable*        m_pParamTable;                         // 0xD4
	TNodeList<TRenderAdapter> m_AdapterList;                         // 0xD8
	TNodeTree<TResource>      m_Resources;                           // 0xE8
	TUINT32                   m_ResourceCount;                       // 0x100
	TUINT32                   m_iFrameCount;                         // 0x104
	TBOOL                     m_bHasDyingResources;                  // 0x108
	TKeyframeLibraryManager   m_KeyframeManager;                     // 0x10C
	TStack<TMatrix44, 20>     m_Transforms;                          // 0x118
};

TSINGLETON_DECLARE_ALIAS( TRender, Render );

TOSHI_NAMESPACE_END
