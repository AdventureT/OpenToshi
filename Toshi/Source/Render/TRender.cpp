#include "ToshiPCH.h"
#include "TRender.h"
#include "TRenderParamTable.h"
#include "Toshi/T2ResourceManager.h"
#include "TModelManager.h"
#include "File/TFile.h"

#ifdef TRENDERINTERFACE_DX11
#  include "Platform/DX11/TRender_DX11.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00690590
TNodeList<TRenderAdapter>* TRender::GetAdapterList()
{
	return &m_AdapterList;
}

// $deBlob: FUNCTION 00690c20
TBOOL TRender::IsInScene()
{
	return m_bInScene;
}

// $deBlob: FUNCTION 00690c30
TBOOL TRender::IsCreated()
{
	return m_bCreated;
}

// $deBlob: FUNCTION 00690c40
TBOOL TRender::IsDisplayCreated()
{
	return m_bDisplayCreated;
}

// $deBlob: FUNCTION 00690c90
TNodeTree<TResource>* TRender::GetResources()
{
	return &m_Resources;
}

// $deBlob: FUNCTION 00691170
TRenderContext* TRender::GetCurrentContext() const
{
	return m_pCurrentContext;
}

// $deBlob: FUNCTION 00691180
// Sets new render context and returns the old one
TRenderContext* TRender::SetCurrentRenderContext( TRenderContext* a_pRenderContext )
{
	return TExchange( m_pCurrentContext, a_pRenderContext );
}

// $deBlob: FUNCTION 0068ff20
// $deBlob: FUNCTION 0068ff30
// $deBlob: FUNCTION 0068ff40
// $deBlob: FUNCTION 0068ff50
// $deBlob: FUNCTION 0068ff60
// $deBlob: FUNCTION 0053eb20
// $deBlob: FUNCTION 00858eb0
TDEFINE_CLASS_NORUNTIME( TRender );

// $deBlob: FUNCTION 0068ff70
// $Barnyard: FUNCTION 006bf490
TRender::TRender()
{
	m_bCreateSystemResources = TTRUE;
	m_bInScene               = TFALSE;
	m_bCreated               = TFALSE;
	m_bDisplayCreated        = TFALSE;
	m_ScreenOffset.Set( 0.0f, 0.0f );
	m_eAspectRatio       = ASPECT_RATIO_4_3;
	m_pCurrentContext    = TNULL;
	m_pCreatedContext    = TNULL;
	m_pUnk0D0            = TNULL;
	m_ResourceCount      = 1;
	m_iFrameCount        = 0;
	m_bHasDyingResources = TFALSE;

	// m_Top = 0, identity
	m_Transforms.Reset();
	m_Transforms.PushNull().Identity();

	new T2ResourceManager( 6000 );
	m_pParamTable = new TRenderParamTable();

	new TModelManager();

	TUtil::MemClear( m_aSysResources, sizeof( m_aSysResources ) );

	m_LightColour.Set(
	    1.0f, 1.0f, 1.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f
	);

	m_LightDirection.Set(
	    -0.47f, -0.74f, 0.47f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f,
	    0.0f, 0.0f, 0.0f, 0.0f
	);
}

// $deBlob: FUNCTION 00690220
// $Barnyard: FUNCTION 006bf6d0
TRender::~TRender()
{
	if ( m_pParamTable )
		delete m_pParamTable;

	if ( TModelManager::GetSingleton() )
		delete TModelManager::GetSingleton();

	if ( T2ResourceManager::GetSingleton() )
		delete T2ResourceManager::GetSingleton();
}

// $deBlob: FUNCTION 00690320
TBOOL TRender::CreateDisplay()
{
	TASSERT( TTRUE == IsCreated() );
	TASSERT( TFALSE == IsDisplayCreated() );

	m_bDisplayCreated = TTRUE;
	return TTRUE;
}

// NOTE: pure virtual with a body, called by TRenderDX11::DestroyDisplay
TBOOL TRender::DestroyDisplay()
{
	FlushDyingResources();

	if ( m_pUnk0D0 )
		m_pUnk0D0->Delete();

	if ( IsDisplayCreated() )
	{
		TASSERT( TTRUE == IsDisplayCreated() );
		m_bDisplayCreated = TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006903e0
// $Barnyard: FUNCTION 006bf850
void TRender::Update( TFLOAT a_fDeltaTime )
{
	FlushDyingResources();
}

// $deBlob: FUNCTION 006903f0
// $Barnyard: FUNCTION 006bea70
void TRender::BeginScene()
{
	TASSERT( TTRUE == IsCreated() );
	TASSERT( TTRUE == IsDisplayCreated() );

	m_iFrameCount += 1;
	// m_Top = 0, identity
	m_Transforms.Reset();
	m_Transforms.PushNull().Identity();
}

// $deBlob: FUNCTION 00690490
void TRender::EndScene()
{
	TASSERT( TTRUE == IsCreated() );
	TASSERT( TTRUE == IsDisplayCreated() );
}

// $deBlob: FUNCTION 00690500
// $Barnyard: FUNCTION 006be9f0
TBOOL TRender::Create()
{
	TASSERT( TFALSE == IsCreated() );
	m_pCreatedContext = CreateRenderContext();

	if ( m_pCreatedContext )
	{
		m_pCurrentContext = m_pCreatedContext;
		m_bCreated        = TTRUE;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00690560
// $Barnyard: FUNCTION 006bea30
TBOOL TRender::Destroy()
{
	m_pCurrentContext = TNULL;

	if ( m_pCreatedContext )
	{
		delete m_pCreatedContext;
		m_pCreatedContext = TNULL;
	}

	m_bCreated = TFALSE;
	return TTRUE;
}

// $deBlob: FUNCTION 006905a0
// $Barnyard: FUNCTION 006becf0
void TRender::DumpStats()
{
	static const TCHAR* s_aDepthStencilFormats[] = { "D16_S0", "D15_S1", "D24_S0", "D24_S8", "D32_S0" };

	TFile* pFile = TFile::Create( "TRender.txt", TFILEMODE_CREATENEW );

	if ( pFile != TNULL )
	{
		pFile->CPrintf( "-\r\n" );

		// NOTE: retail passes the minor version before the major one
		pFile->CPrintf( "Toshi rendering interface through object [\"%s\"] Version : %u.%u\r\n", GetClass()->GetName(), GetClass()->GetVersionMinor(), GetClass()->GetVersionMajor() );
		pFile->CPrintf( "-\r\n" );

		for ( auto pAdapter = m_AdapterList.Begin(); pAdapter != m_AdapterList.End(); pAdapter++ )
		{
			pFile->CPrintf( "Adapter       : %d\r\n", pAdapter->GetAdapterIndex() );
			pFile->CPrintf( "  Driver      : %s\r\n", pAdapter->GetDriver() );
			pFile->CPrintf( "  Description : %s\r\n", pAdapter->GetDriverDescription() );
			pFile->CPrintf( "  Product ID  : %u\r\n", pAdapter->GetProductID() );
			pFile->CPrintf( "  Version     : %u\r\n", pAdapter->GetVersion() );
			pFile->CPrintf( "  Sub-version : %u\r\n", pAdapter->GetSubVersion() );
			pFile->CPrintf( "  Build       : %u\r\n", pAdapter->GetBuild() );
			pFile->CPrintf( "  Supports %d modes:\r\n", pAdapter->GetModeList()->Count() );

			for ( auto pMode = pAdapter->GetModeList()->Begin(); pMode != pAdapter->GetModeList()->End(); pMode++ )
			{
				const TCHAR* sz32Bit = pMode->Is32Bit() == TTRUE ? "Yes" : "No";
				const TCHAR* sz16Bit = pMode->Is16Bit() == TTRUE ? "Yes" : "No";

				pFile->CPrintf( "    Mode %d: %ux%u , %uhz: 16/32 bit Frame Buffer [%s/%s]\r\n", pMode->GetModeIndex(), pMode->GetWidth(), pMode->GetHeight(), pMode->GetRefreshRate(), sz16Bit, sz32Bit );

				for ( TINT i = 0; i < TINT( pAdapter->GetNumSupportedDevices() ); i++ )
				{
					TRenderAdapter::Mode::Device* pDevice = pMode->GetDevice( i );

					pFile->CPrintf( "      Supported Device [\"%s\"]: \r\n", pDevice->GetTypeString() );
					pFile->CPrintf( "        Supports Hardware TnL? [\"%s\"]: \r\n", pDevice->SupportsHardwareTransfomations() == TTRUE ? "Yes!" : "No" );
					pFile->CPrintf( "        Supports Windowed? [\"%s\"]: \r\n", pDevice->CanRenderWindowed() == TTRUE ? "Yes!" : "No" );
					pFile->CPrintf( "        Supported Depth/Stencil Modes : " );

					for ( TINT k = 0; k < TINT( DEPTHSTENCILFORMAT_NUMOF ); k++ )
					{
						if ( pDevice->IsDepthStencilFormatSupported( k ) == TTRUE )
							pFile->CPrintf( "\"%s\" ", s_aDepthStencilFormats[ k ] );
					}

					pFile->CPrintf( "\r\n" );
				}
			}
		}

		pFile->Destroy();
	}
}

// $deBlob: FUNCTION 006908a0
// $Barnyard: FUNCTION 006bf030
TRenderAdapter::Mode::Device* TRender::FindDevice( const DISPLAYPARAMS& a_rDisplayParams )
{
	for ( auto pAdapter = GetAdapterList()->Begin(); pAdapter != GetAdapterList()->End(); pAdapter++ )
	{
		for ( auto pMode = pAdapter->GetModeList()->Begin(); pMode != pAdapter->GetModeList()->End(); pMode++ )
		{
			for ( TINT i = 0; i < TINT( pAdapter->GetNumSupportedDevices() ); i++ )
			{
				auto pDevice = pMode->GetDevice( i );

				if ( pDevice->IsSoftware() ) continue;

				TBOOL bPassedColourDepth = TTRUE;

				if ( a_rDisplayParams.uiColourDepth == 32 )
				{
					if ( !pDevice->GetMode()->Is32Bit() )
						bPassedColourDepth = TFALSE;
				}
				else if ( a_rDisplayParams.uiColourDepth == 16 )
				{
					bPassedColourDepth = pDevice->GetMode()->Is16Bit();
				}

				TBOOL bPassedWindowed = TTRUE;

				if ( a_rDisplayParams.bWindowed == TTRUE )
					bPassedWindowed = pDevice->CanRenderWindowed();

				const TUINT32 uiWidth  = pDevice->GetMode()->GetWidth();
				const TUINT32 uiHeight = pDevice->GetMode()->GetHeight();

				if ( uiHeight < uiWidth || a_rDisplayParams.bWindowed != TTRUE || TINT( a_rDisplayParams.uiWidth ) <= TINT( a_rDisplayParams.uiHeight ) )
				{
					if ( pDevice->GetMode()->GetWidth() != a_rDisplayParams.uiWidth || pDevice->GetMode()->GetHeight() != a_rDisplayParams.uiHeight )
						continue;
				}
				else
				{
					if ( TINT( pDevice->GetMode()->GetWidth() ) < TINT( a_rDisplayParams.uiWidth ) || TINT( pDevice->GetMode()->GetHeight() ) < TINT( a_rDisplayParams.uiHeight ) )
						continue;
				}

				if ( pDevice->IsDepthStencilFormatSupported( a_rDisplayParams.eDepthStencilFormat ) == TTRUE && bPassedColourDepth && bPassedWindowed )
					return pDevice;
			}
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00690b00
TINT TRender::GetDepthStencilFormat( TUINT32 a_uiDepthBits, TUINT32 a_uiStencilBits )
{
	if ( a_uiDepthBits == 16 )
	{
		if ( a_uiStencilBits == 0 ) return DEPTHSTENCILFORMAT_D16;
	}
	else if ( a_uiDepthBits == 15 )
	{
		if ( a_uiStencilBits == 1 ) return DEPTHSTENCILFORMAT_D15_S1;
	}
	else if ( a_uiDepthBits == 24 )
	{
		if ( a_uiStencilBits == 1 ) return DEPTHSTENCILFORMAT_D24_X8;
		if ( a_uiStencilBits == 8 ) return DEPTHSTENCILFORMAT_D24_S8;
	}
	else if ( a_uiDepthBits == 32 && a_uiStencilBits == 0 )
	{
		return DEPTHSTENCILFORMAT_D32;
	}

	return -1;
}

// $deBlob: FUNCTION 00690b70
// $Barnyard: FUNCTION 006be9b0
void TRender::GetScreenOffset( TVector2& a_rVec )
{
	a_rVec.Set( m_ScreenOffset );
}

// $deBlob: FUNCTION 00690b90
// $Barnyard: FUNCTION 006be9d0
void TRender::SetScreenOffset( const TVector2& a_rVec )
{
	m_ScreenOffset.Set( a_rVec );
}

// $deBlob: FUNCTION 00690bb0
TFLOAT TRender::GetResolutionScalar()
{
	DISPLAYPARAMS* pDisplayParams = GetCurrentDisplayParams();

	if ( m_eAspectRatio == ASPECT_RATIO_16_9 )
		return ( TFLOAT( TINT( pDisplayParams->uiHeight ) ) * 16.0f ) / ( TFLOAT( TINT( pDisplayParams->uiWidth ) ) * 9.0f );

	return ( TFLOAT( TINT( pDisplayParams->uiHeight ) ) * 4.0f ) / ( TFLOAT( TINT( pDisplayParams->uiWidth ) ) * 3.0f );
}

// $deBlob: FUNCTION 00690ca0
// $Barnyard: FUNCTION 006bf240
TResource* TRender::CreateResource( const TClass* a_pClass, const TCHAR* a_szName, TResource* a_pParent )
{
	TASSERT( TNULL != a_pClass );
	TASSERT( a_pClass->IsA( &TGetClass( TResource ) ) );

	TResource* pResource = TSTATICCAST( TResource, a_pClass->CreateObject() );
	TASSERT( TNULL != pResource );

	if ( pResource == TNULL )
		return TNULL;

	m_Resources.Remove( *pResource, TFALSE );

	if ( a_pParent == TNULL )
		a_pParent = m_Resources.GetRoot();

	m_Resources.Insert( a_pParent, pResource );
	pResource->SetUId( m_ResourceCount++ );
	pResource->SetRenderer( this );
	pResource->SetName( a_szName );

	return pResource;
}

// $deBlob: FUNCTION 00690dc0
TResource* TRender::CreateResourceInPlace( const TClass* a_pClass, void* a_pMemory, const TCHAR* a_szName, TResource* a_pParent )
{
	TASSERT( TNULL != a_pClass );
	TASSERT( a_pClass->IsA( &TGetClass( TResource ) ) );

	TResource* pResource = TSTATICCAST( TResource, a_pClass->CreateObjectInPlace( a_pMemory ) );
	TASSERT( TNULL != pResource );

	if ( pResource == TNULL )
		return TNULL;

	m_Resources.Remove( *pResource, TFALSE );

	if ( a_pParent == TNULL )
		a_pParent = m_Resources.GetRoot();

	m_Resources.Insert( a_pParent, pResource );
	pResource->SetUId( m_ResourceCount++ );
	pResource->SetRenderer( this );
	pResource->SetName( a_szName );
	pResource->AddState( TResourceState_External );

	return pResource;
}

// Moves a_pResource to the end of its parent's children
// $deBlob: FUNCTION 00690ee0
void TRender::SetAsLastChild( TResource* a_pResource )
{
	if ( a_pResource != TNULL && a_pResource->GetParent() != TNULL )
	{
		TResource* pParent = a_pResource->GetParent();
		m_Resources.Remove( *a_pResource, TFALSE );

		if ( pParent == TNULL )
			pParent = m_Resources.GetRoot();

		m_Resources.Insert( pParent, a_pResource );
	}
}

// $deBlob: FUNCTION 00690f50
// $Barnyard: FUNCTION 006be8f0
void TRender::SetResourceExplicit( TResource* a_pResource, SYSRESOURCE a_eResource )
{
	// NOTE: retail's assert allows SYSRESOURCE_NUMOF
	TASSERT( a_eResource >= 0 && a_eResource <= SYSRESOURCE_NUMOF, "Unknown resource" );
	TASSERT( TNULL == m_aSysResources[ a_eResource ], "This resource has already been assigned!" );
	m_aSysResources[ a_eResource ] = a_pResource;
}

// $deBlob: FUNCTION 00690fe0
// $Barnyard: FUNCTION 006bec20
void TRender::DestroyResource( TResource* a_pResource )
{
	TASSERT( TNULL != a_pResource->Tree() );
	TASSERT( 0 == ( a_pResource->m_State & TResourceState_Dead ) );

	if ( !a_pResource->IsDying() )
	{
		m_bHasDyingResources = TTRUE;
		a_pResource->AddState( TResourceState_Dying );
		DestroyResourceRecurse( a_pResource->Child() );
		a_pResource->Invalidate();
	}
}

// $deBlob: FUNCTION 00691080
// $Barnyard: FUNCTION 006bf7d0
void TRender::FlushDyingResources()
{
	if ( m_bHasDyingResources == TTRUE )
		DestroyDyingResources();
}

// $deBlob: FUNCTION 006910a0
void TRender::DestroyDyingResources()
{
	do
	{
		m_bHasDyingResources = TFALSE;
		DestroyDyingResources( m_Resources.ChildOfRoot() );
	} while ( m_bHasDyingResources == TTRUE );
}

// $deBlob: FUNCTION 00691190
void TRender::FlushShaders()
{
	TASSERT( TTRUE == IsInScene() );
	TRenderDX11::Interface()->FlushOrderTables();
}

// $deBlob: FUNCTION 006911d0
TBOOL TRender::CreateSystemResources()
{
	return TTRUE;
}

// $deBlob: FUNCTION 006911e0
// $deBlob: FUNCTION 00691200
// $Barnyard: FUNCTION 006bf800
void TRender::DestroySystemResources()
{
	DestroyResourceRecurse( m_Resources.ChildOfRoot() );
	FlushDyingResources();
}

// $deBlob: FUNCTION 00691220
// $Barnyard: FUNCTION 006bf750
void TRender::DestroyDyingResources( TResource* a_pResources )
{
	// The resources are a circular list of siblings starting at a_pResources
	TResource* pFirst    = a_pResources;
	TResource* pResource = a_pResources;

	while ( pResource )
	{
		TResource* pNext = pResource->Next();
		if ( pNext == pFirst ) pNext = TNULL;

		if ( !pResource->IsDying() )
		{
			if ( TResource* pChild = pResource->Child() )
				DestroyDyingResources( pChild );
		}
		else
		{
			// The list starts at the next resource once this one is gone
			if ( pResource == pFirst )
				pFirst = pNext;

			DeleteResourceRecurse( pResource->Child() );
			DeleteResourceAtomic( pResource );
		}

		pResource = pNext;
	}
}

// $deBlob: FUNCTION 00691280
// $Barnyard: FUNCTION 006beaa0
void TRender::DestroyResourceRecurse( TResource* a_pResource )
{
	if ( a_pResource == TNULL )
		return;

	TResource* pLast     = a_pResource->Prev();
	TResource* pResource = a_pResource;

	do
	{
		TResource* pNext = ( pResource != pLast ) ? pResource->Next() : TNULL;

		if ( !pResource->IsDying() )
		{
			m_bHasDyingResources = TTRUE;
			pResource->AddState( TResourceState_Dying );

			if ( pResource->Child() )
				DestroyResourceRecurse( pResource->Child() );

			pResource->Invalidate();
		}

		pResource = pNext;
	} while ( pResource != TNULL );
}

// $deBlob: FUNCTION 006912f0
void TRender::DeleteResource( TResource* a_pResource )
{
	DeleteResourceRecurse( a_pResource->Child() );
	DeleteResourceAtomic( a_pResource );
}

// $deBlob: FUNCTION 00691310
void TRender::DeleteResourceRecurse( TResource* a_pResource )
{
	while ( a_pResource )
	{
		TResource* pNext = a_pResource->Next();
		if ( pNext == a_pResource ) pNext = TNULL;

		// NOTE: DeleteResourceAtomic inlined
		if ( a_pResource->Child() )
			DeleteResourceRecurse( a_pResource->Child() );

		TASSERT( TFALSE == a_pResource->IsValid() );

		if ( a_pResource->IsCreated() )
			a_pResource->OnDestroy();

		m_Resources.Remove( a_pResource, TFALSE );

		if ( a_pResource->IsExternal() )
			a_pResource->~TResource();
		else
			delete a_pResource;

		a_pResource = pNext;
	}
}

// $deBlob: FUNCTION 006913c0
// $Barnyard: FUNCTION 006bf420
void TRender::DeleteResourceAtomic( TResource* a_pResource )
{
	if ( a_pResource == TNULL )
		return;

	TResource* pChild = a_pResource->Child();

	while ( pChild )
	{
		TResource* pNext = pChild->Next();
		if ( pNext == pChild ) pNext = TNULL;

		DeleteResourceAtomic( pChild );
		pChild = pNext;
	}

	TASSERT( TFALSE == a_pResource->IsValid() );

	if ( a_pResource->IsCreated() )
		a_pResource->OnDestroy();

	m_Resources.Remove( a_pResource, TFALSE );

	if ( a_pResource->IsExternal() )
		a_pResource->~TResource();
	else
		delete a_pResource;
}

// $deBlob: FUNCTION 00691480
void TRender::DestroyResourcesOfClassRecurse( const TClass* a_pClass, TResource* a_pResource )
{
	TResource* pResource = a_pResource;

	while ( pResource )
	{
		TResource* pNext = pResource->Next();
		if ( pNext == a_pResource ) pNext = TNULL;

		if ( pResource->Child() )
			DestroyResourcesOfClassRecurse( a_pClass, pResource->Child() );

		if ( !pResource->IsDying() && pResource->GetClass()->IsA( a_pClass ) )
			DestroyResource( pResource );

		pResource = pNext;
	}
}

// $deBlob: FUNCTION 00691f80
// $Barnyard: FUNCTION 006bf6b0
void TRender::SetLightColourMatrix( const TMatrix44& a_rMatrix )
{
	m_LightColour = a_rMatrix;
}

// $deBlob: FUNCTION 00691fc0
// $Barnyard: FUNCTION 006bf690
void TRender::SetLightDirectionMatrix( const TMatrix44& a_rMatrix )
{
	m_LightDirection = a_rMatrix;
}

TOSHI_NAMESPACE_END
