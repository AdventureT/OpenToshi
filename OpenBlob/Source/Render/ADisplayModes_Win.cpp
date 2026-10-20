#include "pch.h"
#include "ADisplayModes_Win.h"

#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TRenderAdapter_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005eb2f0
// $deBlob: FUNCTION 005eb300
// $deBlob: FUNCTION 005eb380
// $deBlob: FUNCTION 005eb400
// $deBlob: FUNCTION 005eb410
// $deBlob: FUNCTION 0053ca20
// $deBlob: FUNCTION 008585f0
TDEFINE_CLASS( ADisplayModes_Win, 1, 1 );

// $deBlob: FUNCTION 005eb440
// $deBlob: FUNCTION 005ebc00
ADisplayModes_Win::~ADisplayModes_Win()
{
}

// $deBlob: FUNCTION 005eb450
void ADisplayModes_Win::Initialise()
{
	TNodeList<TRenderAdapter>* pAdapterList = TRender::GetSingleton()->GetAdapterList();

	TUtil::Log( "Enum Display Modes" );
	TUtil::LogDown();

	// NOTE: the output of the previous mode (to log each display once); retail leaves it uninitialised
	DXGI_OUTPUT_DESC oLastOutput;
#ifdef DEBLOB_COMMUNITY_PATCH
	TUtil::MemClear( &oLastOutput, sizeof( oLastOutput ) );
#endif

	T2_FOREACH( *pAdapterList, itAdapter )
	{
		TD3DAdapter* pAdapter = TSTATICCAST( TD3DAdapter, itAdapter.Get() );

		TUtil::Log( "Adapter: %s", pAdapter->GetDriverDescription() );
		TUtil::LogDown();

		T2_FOREACH( *pAdapter->GetModeList(), itMode )
		{
			TD3DAdapter::Mode* pMode = TSTATICCAST( TD3DAdapter::Mode, itMode.Get() );

			TUINT32 uiWidth  = pMode->GetWidth();
			TUINT32 uiHeight = pMode->GetHeight();

			// NOTE: the engine's "display name" of the mode is the DXGI_OUTPUT_DESC of its output
			const DXGI_OUTPUT_DESC* pOutput = TREINTERPRETCAST( const DXGI_OUTPUT_DESC*, pMode->GetDisplayName() );

			if ( memcmp( pOutput, &oLastOutput, sizeof( DXGI_OUTPUT_DESC ) ) != 0 )
			{
				oLastOutput = *pOutput;

				TCHAR szDeviceName[ 128 ];
				TUtil::Log( "" );
				TUtil::Log(
				    "Display: %s (%dx%d)",
				    TStringManager::StringUnicodeToChar( szDeviceName, oLastOutput.DeviceName, -1 ),
				    oLastOutput.DesktopCoordinates.right - oLastOutput.DesktopCoordinates.left,
				    oLastOutput.DesktopCoordinates.bottom - oLastOutput.DesktopCoordinates.top
				);
				TUtil::Log( "" );
			}

			const DXGI_RATIONAL& rRefreshRate = pMode->GetDescription()->RefreshRate;

			TUtil::Log(
			    "DisplayMode %d x %d (%.4f Hz) = [%d / %d]",
			    uiWidth,
			    uiHeight,
			    TFLOAT( rRefreshRate.Numerator ) / TFLOAT( rRefreshRate.Denominator ),
			    rRefreshRate.Numerator,
			    rRefreshRate.Denominator
			);

			TUINT32 uiColourDepth = 32;

			if ( !DoesModeExist( uiWidth, uiHeight, uiColourDepth ) )
			{
				m_Modes.PushBack( { uiWidth, uiHeight, uiColourDepth } );
			}
		}

		TUtil::LogUp();
	}

	TUtil::LogUp();
}

// $deBlob: FUNCTION 005eb7a0
TBOOL ADisplayModes_Win::DoesModeExist( const TUINT32& a_rWidth, const TUINT32& a_rHeight, const TUINT32& a_rColourDepth )
{
	T2_FOREACH( m_Modes, it )
	{
		if ( it->m_uiWidth == a_rWidth && it->m_uiHeight == a_rHeight && it->m_uiColourDepth == a_rColourDepth )
			return TTRUE;
	}

	return TFALSE;
}
