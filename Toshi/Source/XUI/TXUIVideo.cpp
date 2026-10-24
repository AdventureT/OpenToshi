#include "ToshiPCH.h"
#include "TXUIVideo.h"
#include "TXUIMovie.h"
#include "TXUIRenderer.h"
#include "TXUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiVideo" type info
const TXUITypeInfo XURXUIVideoData::sm_sTypeInfo = { L"XuiVideo", &XURXUIElementData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUIVideoData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006bbb50
const TXUITypeInfo* XURXUIVideoData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006bbb60
XURXUIVideoData::XURXUIVideoData()
{
	m_uiSizeMode = 0;
	m_fVolume    = -12.0f;
	m_uiFile     = 0;
	m_bPause     = TFALSE;
	m_bLoop      = TFALSE;

	m_pClass = TClass::Find( "TXUIVideo", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006bbba0
TBOOL XURXUIVideoData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006bbbd0
TUINT32 XURXUIVideoData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006bbc00
TBOOL XURXUIVideoData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006bbc70
TBOOL XURXUIVideoData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "File", -1 ) == 0 )
	{
		a_rPropIndex = Prop_File;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "SizeMode", -1 ) == 0 )
	{
		a_rPropIndex = Prop_SizeMode;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Pause", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Pause;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Loop", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Loop;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Volume", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Volume;
		return TTRUE;
	}

	// NOTE: retail doesn't increment a_rObjectIndex here (the other data classes do)
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006bbd40
TUINT32 XURXUIVideoData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		// NOTE: Loop is a bool too but retail returns 4 for it
		if ( a_uiPropIndex == Prop_Pause )
			return 1;

		return ( a_uiPropIndex != Prop_File ) ? 4 : 2;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006bbdc0
TBOOL XURXUIVideoData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_Volume ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006bbe30
TBOOL XURXUIVideoData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006bbe90
TBOOL XURXUIVideoData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT8 uiNumProps = *a_rData;
	a_rData += 1;

	if ( uiNumProps != 0 )
	{
		TUINT8 uiPropMask = 0;

		if ( m_uiPropLevelMask > 0 )
		{
			uiPropMask = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_File ) )
		{
			m_uiFile = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_SizeMode ) )
		{
			m_uiSizeMode = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Pause ) )
		{
			m_bPause = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_Loop ) )
		{
			m_bLoop = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_Volume ) )
		{
			m_fVolume = PARSEFLOAT_BIG( a_rData );
			a_rData += 4;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUIVideo
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006bbf60
// $deBlob: FUNCTION 006bbf70
// $deBlob: FUNCTION 006bbfd0
// $deBlob: FUNCTION 006bc020
// $deBlob: FUNCTION 006bc030
// $deBlob: FUNCTION 0053edb0
// $deBlob: FUNCTION 00859010
TXUI_DEFINE_CLASS( TXUIVideo );

// $deBlob: FUNCTION 006bc040
TXUIVideo::TXUIVideo()
{
	m_bPause  = TFALSE;
	m_bHidden = TFALSE;
	m_bLoop   = TFALSE;
	m_fVolume = 1.0f;
	m_pMovie  = TNULL;
	m_uiFile  = 0;
}

// $deBlob: FUNCTION 006bc080
// $deBlob: FUNCTION 006bc750
// $deBlob: FUNCTION 006bc73b
TXUIVideo::~TXUIVideo()
{
	if ( m_pMovie != TNULL )
		delete m_pMovie;
}

// $deBlob: FUNCTION 006bc0b0
TBOOL TXUIVideo::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	TXUIElement::Create( a_rResource, a_pData, a_bCreateChildren );

	XURXUIVideoData* pVideoData = TSTATICCAST( XURXUIVideoData, a_pData );

	m_bPause  = pVideoData->m_bPause == TTRUE;
	m_bLoop   = pVideoData->m_bLoop == TTRUE;
	m_fVolume = TMath::Pow( 10.0f, pVideoData->m_fVolume / 20.0f );
	TMath::Clip( m_fVolume, 0.0f, 1.0f );

	m_pMovie = TXUI::GetSingleton()->CreateMovie();

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail sets the movie's video before checking it, crashing without a TXUI::CreateMovie callback
	if ( m_pMovie != TNULL )
		m_pMovie->SetVideo( this );
#else
	m_pMovie->SetVideo( this );
#endif

	if ( m_pMovie != TNULL && pVideoData->m_uiFile != 0 )
	{
		m_uiFile   = pVideoData->m_uiFile;
		TBOOL bRes = m_pMovie->Open( a_rResource.GetString( pVideoData->m_uiFile ), TNULL, TFALSE, 1.0f );
		TASSERT( TTRUE == bRes );

		TFLOAT fWidth  = m_pMovie->GetWidth();
		TFLOAT fHeight = m_pMovie->GetHeight();
		m_iWidth       = PackFloat( fWidth );
		m_iHeight      = PackFloat( fHeight );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006bc1f0
void TXUIVideo::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	TFLOAT fScalar = TFLOAT( TMath::FloorToInt( a_fScalar ) );

	if ( a_uiPropIndex == XURXUIVideoData::Prop_File )
	{
		TUINT32 uiFile = a_uiValue1;

		if ( fScalar != 0.0f )
		{
			uiFile = a_uiValue2;
			TASSERT( a_fScalar == 1.0f );
		}

		if ( m_pMovie != TNULL )
		{
			if ( uiFile == 0 )
			{
				m_pMovie->Close();
				return;
			}

			if ( uiFile != m_uiFile )
			{
				m_uiFile   = uiFile;
				TBOOL bRes = m_pMovie->Open( a_pResource->GetString( TUINT16( uiFile ) ), TNULL, TFALSE, 1.0f );
				TASSERT( TTRUE == bRes );

				TFLOAT fWidth  = m_pMovie->GetWidth();
				TFLOAT fHeight = m_pMovie->GetHeight();
				m_iWidth       = PackFloat( fWidth );
				m_iHeight      = PackFloat( fHeight );
			}
		}
	}
	else if ( a_uiPropIndex == XURXUIVideoData::Prop_Pause )
	{
		TUINT32 uiPause = a_uiValue1;

		if ( fScalar != 0.0f )
		{
			uiPause = a_uiValue2;
			TASSERT( a_fScalar == 1.0f );
		}

		if ( ( uiPause == 1 ) != m_bPause )
		{
			// NOTE: retail passes the previous pause state to the movie (so it pauses when unpausing and vice versa)
			if ( m_pMovie != TNULL )
				m_pMovie->SetPaused( m_bPause );

			m_bPause = uiPause == 1;
		}
	}
	else if ( a_uiPropIndex == XURXUIVideoData::Prop_Loop )
	{
		if ( fScalar != 0.0f )
		{
			TASSERT( a_fScalar == 1.0f );
			m_bLoop = a_uiValue2 == 1;
			return;
		}

		m_bLoop = a_uiValue1 == 1;
	}
}

// $deBlob: FUNCTION 006bc4b0
void TXUIVideo::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
		return;
	}

	if ( a_uiPropIndex == XURXUIVideoData::Prop_Volume )
	{
		m_fVolume = TMath::Pow( 10.0f, TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) / 20.0f );

		if ( m_pMovie != TNULL )
			m_pMovie->SetVolume( m_fVolume );
	}
}

// $deBlob: FUNCTION 006bc5b0
void TXUIVideo::Render()
{
	if ( m_pMovie != TNULL )
	{
		TXUI::GetSingleton()->GetRenderer()->SetColour( m_uiColour );

		TFLOAT fHeight = GetHeight();
		TFLOAT fWidth  = GetWidth();
		m_pMovie->Render( fWidth, fHeight );
	}

	T2GUIElement::Render();
}

// $deBlob: FUNCTION 006bc600
void TXUIVideo::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pMovie != TNULL )
	{
		if ( IsVisible() )
		{
			// Pause the movie while a parent is hidden
			TBOOL bVisible = TTRUE;

			for ( T2GUIElement* pParent = GetParent(); pParent != TNULL; pParent = pParent->GetParent() )
			{
				if ( !pParent->IsVisible() )
				{
					bVisible = TFALSE;
					break;
				}
			}

			if ( !bVisible )
			{
				if ( !m_bHidden )
					m_pMovie->SetPaused( TTRUE );
			}
			else if ( m_bHidden == TTRUE )
			{
				m_pMovie->SetPaused( TFALSE );
			}

			m_bHidden = !bVisible;
		}

		m_pMovie->Update( a_fDeltaTime );
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006bc690
void TXUIVideo::SetVisible( TBOOL a_bVisible )
{
	// NOTE: retail compares the visibility with the hidden state, so this only reapplies the current state
	if ( m_pMovie != TNULL && a_bVisible != m_bHidden )
	{
		if ( !a_bVisible )
			m_pMovie->SetPaused( TTRUE );
		else
			m_pMovie->SetPaused( m_bPause );

		m_bHidden = !a_bVisible;
	}

	T2GUIElement::SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 006bc6e0
void TXUIVideo::OnMovieFinished()
{
	if ( !m_bLoop )
		m_pMovie->Close();
}

TOSHI_NAMESPACE_END
