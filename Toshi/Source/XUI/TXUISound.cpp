#include "ToshiPCH.h"
#include "TXUISound.h"
#include "TXUIAudio.h"
#include "TXUI.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// "XuiSound" type info
const TXUITypeInfo XURXUISoundData::sm_sTypeInfo = { L"XuiSound", &XURXUIElementData::sm_sTypeInfo };

// "XuiSoundXACT" type info
const TXUITypeInfo XURXUISoundXACTData::sm_sTypeInfo = { L"XuiSoundXACT", &XURXUISoundData::sm_sTypeInfo };

// "XuiSoundXAudio" type info
const TXUITypeInfo XURXUISoundXAudioData::sm_sTypeInfo = { L"XuiSoundXAudio", &XURXUISoundData::sm_sTypeInfo };

//-----------------------------------------------------------------------------
// XURXUISoundData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e6600
const TXUITypeInfo* XURXUISoundData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// $deBlob: FUNCTION 006e6610
XURXUISoundData::XURXUISoundData()
{
	m_eState  = State_Play;
	m_bFinish = TFALSE;
	m_bLoop   = TFALSE;
	m_fVolume = -12.0f;

	m_pClass = TClass::Find( "TXUISound", &TGetClass( TXUIElement ) );
}

// $deBlob: FUNCTION 006e6650
TBOOL XURXUISoundData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUIElementData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// $deBlob: FUNCTION 006e6680
TUINT32 XURXUISoundData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUIElementData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e66b0
TBOOL XURXUISoundData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6710
TBOOL XURXUISoundData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "State", -1 ) == 0 )
	{
		a_rPropIndex = Prop_State;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "Finish", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Finish;
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

	a_rObjectIndex++;
	return XURXUIElementData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e67c0
TUINT32 XURXUISoundData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_Finish || a_uiPropIndex == Prop_Loop )
			return 1;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6840
TBOOL XURXUISoundData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_Volume ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e68b0
TBOOL XURXUISoundData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUIElementData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6910
TBOOL XURXUISoundData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_State ) )
		{
			m_eState = PARSEDWORD_BIG( a_rData );
			a_rData += 4;
		}

		if ( uiPropMask & BITFLAG( Prop_Loop ) )
		{
			m_bLoop = *a_rData;
			a_rData += 1;
		}

		if ( uiPropMask & BITFLAG( Prop_Finish ) )
		{
			m_bFinish = *a_rData;
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
// XURXUISoundXACTData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e69c0
const TXUITypeInfo* XURXUISoundXACTData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// NOTE: the XURXUISoundData constructor is inlined
// $deBlob: FUNCTION 006e69d0
XURXUISoundXACTData::XURXUISoundXACTData()
    : XURXUISoundData()
{
	m_uiCue       = 0;
	m_uiSoundBank = 0;
	m_uiWaveBank  = 0;
}

// NOTE: XURXUISoundData::Load is inlined
// $deBlob: FUNCTION 006e6a20
TBOOL XURXUISoundXACTData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUISoundData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// NOTE: XURXUISoundData::CalcPileSize is inlined
// $deBlob: FUNCTION 006e6a50
TUINT32 XURXUISoundXACTData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUISoundData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e6a90
TBOOL XURXUISoundXACTData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex < Prop_NUMOF ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6b00
TBOOL XURXUISoundXACTData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "Cue", -1 ) == 0 )
	{
		a_rPropIndex = Prop_Cue;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "SoundBank", -1 ) == 0 )
	{
		a_rPropIndex = Prop_SoundBank;
		return TTRUE;
	}

	if ( TStringManager::String8Compare( a_szName, "WaveBank", -1 ) == 0 )
	{
		a_rPropIndex = Prop_WaveBank;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUISoundData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e6b90
TUINT32 XURXUISoundXACTData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
	{
		if ( a_uiPropIndex == Prop_Cue || a_uiPropIndex == Prop_SoundBank || a_uiPropIndex == Prop_WaveBank )
			return 2;

		return 4;
	}

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6c10
TBOOL XURXUISoundXACTData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6c70
TBOOL XURXUISoundXACTData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6cd0
TBOOL XURXUISoundXACTData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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

		if ( uiPropMask & BITFLAG( Prop_Cue ) )
		{
			m_uiCue = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_SoundBank ) )
		{
			m_uiSoundBank = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}

		if ( uiPropMask & BITFLAG( Prop_WaveBank ) )
		{
			m_uiWaveBank = PARSEWORD_BIG( a_rData );
			a_rData += 2;
		}
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// XURXUISoundXAudioData
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006e6d50
const TXUITypeInfo* XURXUISoundXAudioData::GetTypeInfo() const
{
	return &sm_sTypeInfo;
}

// NOTE: the XURXUISoundData constructor is inlined
// $deBlob: FUNCTION 006e6d60
XURXUISoundXAudioData::XURXUISoundXAudioData()
    : XURXUISoundData()
{
	m_uiFile = 0;
}

// NOTE: XURXUISoundData::Load and LoadData are inlined
// $deBlob: FUNCTION 006e6db0
TBOOL XURXUISoundXAudioData::Load( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	XURXUISoundData::Load( a_rResource, a_rData );
	return LoadData( a_rResource, a_rData );
}

// NOTE: XURXUISoundData::CalcPileSize and LoadData are inlined
// $deBlob: FUNCTION 006e6e10
TUINT32 XURXUISoundXAudioData::CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData )
{
	TUINT32 uiSize = XURXUISoundData::CalcPileSize( a_rResource, a_rData );
	LoadData( a_rResource, a_rData );
	return uiSize;
}

// $deBlob: FUNCTION 006e6e80
TBOOL XURXUISoundXAudioData::ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex == Prop_File ) ? TTRUE : TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::ValidateTimelineProp( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6ef0
TBOOL XURXUISoundXAudioData::TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex )
{
	if ( TStringManager::String8Compare( a_szName, "File", -1 ) == 0 )
	{
		a_rPropIndex = Prop_File;
		return TTRUE;
	}

	a_rObjectIndex++;
	return XURXUISoundData::TranslateTimelineProp( a_szName, a_rObjectIndex, a_rPropIndex );
}

// $deBlob: FUNCTION 006e6f40
TUINT32 XURXUISoundXAudioData::GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return ( a_uiPropIndex != Prop_File ) ? 4 : 2;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::GetTimelinePropSize( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e6fb0
TBOOL XURXUISoundXAudioData::IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::IsFloatPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e7010
TBOOL XURXUISoundXAudioData::IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex )
{
	if ( a_uiObjectIndex == 0 )
		return TFALSE;

	TASSERT( a_uiObjectIndex > 0 );
	return XURXUISoundData::IsColourPropType( a_uiObjectIndex - 1, a_uiPropIndex );
}

// $deBlob: FUNCTION 006e7070
TBOOL XURXUISoundXAudioData::LoadData( TXUIResource& a_rResource, TUINT8*& a_rData )
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
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// TXUISound
//-----------------------------------------------------------------------------

// Looks up the sub sound named after the file name (without its path, lower case) of a File property
// Create takes the name with TString8::Mid, UpdateTimelineProp with TString8::Right (a_bRight): same result
static void* GetSubSoundFromFile( const TWCHAR* a_wszFile, TBOOL a_bRight )
{
	TString8 strName( a_wszFile );

	TINT iNameStart = strName.FindReverse( '\\', -1 ) + 1;

	if ( iNameStart == 0 )
		iNameStart = strName.FindReverse( '/', -1 ) + 1;

	if ( a_bRight )
		strName = strName.Right( strName.Length() - iNameStart );
	else
		strName = strName.Mid( iNameStart, strName.Length() - iNameStart );
	strName.MakeLower();

	return TXUI::GetSingleton()->GetAudio()->GetSubSound( strName.GetString() );
}

// $deBlob: FUNCTION 006e70c0
// $deBlob: FUNCTION 006e70d0
// $deBlob: FUNCTION 006e7130
// $deBlob: FUNCTION 006e7190
// $deBlob: FUNCTION 006e71a0
// $deBlob: FUNCTION 0053f6b0
// $deBlob: FUNCTION 00859390
TXUI_DEFINE_CLASS( TXUISound );

// $deBlob: FUNCTION 006e71b0
TXUISound::TXUISound()
{
	m_bXAudio  = TFALSE;
	m_uiFile   = 0;
	m_eState   = XURXUISoundData::State_Play;
	m_pSound   = TNULL;
	m_pChannel = TNULL;
	m_fVolume  = 1.0f;
}

// $deBlob: FUNCTION 006e71f0
// $deBlob: FUNCTION 006e78b0
// $deBlob: FUNCTION 006e789b
TXUISound::~TXUISound()
{
}

// $deBlob: FUNCTION 006e7210
TBOOL TXUISound::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	// NOTE: retail always creates the children
	TXUIElement::Create( a_rResource, a_pData, TTRUE );

	if ( TStringManager::String16Compare( a_rResource.GetString( a_pData->GetTypeIndex() ), L"XuiSoundXAudio", -1 ) == 0 )
	{
		XURXUISoundXAudioData* pSoundData = TSTATICCAST( XURXUISoundXAudioData, a_pData );

		m_bXAudio = TTRUE;
		m_eState  = pSoundData->m_eState;
		m_fVolume = TMath::Pow( 10.0f, pSoundData->m_fVolume / 20.0f );
		m_uiFile  = pSoundData->m_uiFile;
		m_pSound  = GetSubSoundFromFile( a_rResource.GetString( TUINT16( m_uiFile ) ), TFALSE );

		if ( m_pSound != TNULL && m_eState == XURXUISoundData::State_Play )
			TXUI::GetSingleton()->GetAudio()->PlaySound( m_pSound, m_fVolume, &m_pChannel );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006e7360
void TXUISound::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar )
{
	if ( a_uiObjectIndex == 0 )
	{
		// XuiSoundXAudio
		if ( m_bXAudio == TTRUE && a_uiPropIndex == XURXUISoundXAudioData::Prop_File )
		{
			TUINT32 uiFile = a_uiValue1;

			if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) != 0.0f )
			{
				TASSERT( a_fScalar == 1.0f );
				uiFile = a_uiValue2;
			}

			if ( uiFile == m_uiFile )
				return;

			if ( uiFile != 0 )
			{
				void* pSound = GetSubSoundFromFile( a_pResource->GetString( TUINT16( uiFile ) ), TTRUE );

				if ( pSound != m_pSound )
				{
					m_pSound = pSound;

					if ( m_eState == XURXUISoundData::State_Play )
					{
						TXUI::GetSingleton()->GetAudio()->Stop( m_pChannel );
						TXUI::GetSingleton()->GetAudio()->PlaySound( m_pSound, m_fVolume, &m_pChannel );
					}
				}

				m_uiFile = uiFile;
				return;
			}

			m_pSound = TNULL;
			m_uiFile = 0;
		}

		return;
	}

	TASSERT( a_uiObjectIndex > 0 );
	a_uiObjectIndex--;

	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_uiValue1, a_uiValue2, a_fScalar );
		return;
	}

	// XuiSound
	if ( a_uiPropIndex == XURXUISoundData::Prop_State )
	{
		TUINT32 eState = a_uiValue1;

		if ( TFLOAT( TMath::FloorToInt( a_fScalar ) ) != 0.0f )
		{
			eState = a_uiValue2;
			TASSERT( a_fScalar == 1.0f );
		}

		m_eState = eState;

		if ( eState == XURXUISoundData::State_Play )
		{
			if ( m_pChannel == TNULL && m_pSound != TNULL )
				TXUI::GetSingleton()->GetAudio()->PlaySound( m_pSound, m_fVolume, &m_pChannel );
		}
		else if ( eState == XURXUISoundData::State_Stop && m_pChannel != TNULL )
		{
			TXUI::GetSingleton()->GetAudio()->Stop( m_pChannel );
			m_pChannel = TNULL;
		}
	}
}

// $deBlob: FUNCTION 006e7750
void TXUISound::UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar )
{
	// NOTE: no XuiSoundXAudio/XuiSoundXACT float property
	if ( a_uiObjectIndex == 0 )
		return;

	a_uiObjectIndex--;

	if ( a_uiObjectIndex != 0 )
	{
		TASSERT( a_uiObjectIndex > 0 );
		TXUIElement::UpdateTimelineProp( a_pResource, a_uiObjectIndex - 1, a_uiPropIndex, a_fValue1, a_fValue2, a_fScalar );
		return;
	}

	// XuiSound
	if ( a_uiPropIndex == XURXUISoundData::Prop_Volume )
	{
		m_fVolume = TMath::Pow( 10.0f, TMath::LERP( a_fValue1, a_fValue2, a_fScalar ) / 20.0f );

		if ( m_pChannel != TNULL )
			TXUI::GetSingleton()->GetAudio()->SetVolume( m_pChannel, m_fVolume );
	}
}

TOSHI_NAMESPACE_END
