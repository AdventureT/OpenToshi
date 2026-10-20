#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// XuiSound (base of XuiSoundXACT and XuiSoundXAudio, never created itself)
//-----------------------------------------------------------------------------
class XURXUISoundData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_State,
		Prop_Loop,
		Prop_Finish,
		Prop_Volume,
		Prop_NUMOF,
	};

	// Values of Prop_State
	enum State : TUINT32
	{
		State_Play = 2,
		State_Stop = 4,
	};

public:
	XURXUISoundData();

	// $deBlob: FUNCTION 006e78e0
	// $deBlob: FUNCTION 006e7870
	~XURXUISoundData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                    // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                            // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                 // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                  // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                      // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                     // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                    // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TUINT32 m_eState;  // 0x30 State_Play by default
	TBOOL   m_bFinish; // 0x34
	TBOOL   m_bLoop;   // 0x35
	TFLOAT  m_fVolume; // 0x38 dB, -12 by default
};

class XURXUISoundXACTData : public XURXUISoundData
{
public:
	enum Prop
	{
		Prop_Cue,
		Prop_SoundBank,
		Prop_WaveBank,
		Prop_NUMOF,
	};

public:
	XURXUISoundXACTData();

	// $deBlob: FUNCTION 006e7910
	// $deBlob: FUNCTION 006e7880
	~XURXUISoundXACTData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                    // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                            // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                 // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                  // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                      // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                     // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                    // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TUINT16 m_uiCue;       // 0x3C string index
	TUINT16 m_uiSoundBank; // 0x3E string index
	TUINT16 m_uiWaveBank;  // 0x40 string index
};

class XURXUISoundXAudioData : public XURXUISoundData
{
public:
	enum Prop
	{
		Prop_File,
		Prop_NUMOF,
	};

public:
	XURXUISoundXAudioData();

	// $deBlob: FUNCTION 006e7940
	// $deBlob: FUNCTION 006e7890
	~XURXUISoundXAudioData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                    // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                            // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                 // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                  // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                      // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                     // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                    // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TUINT16 m_uiFile; // 0x3C string index
};

//-----------------------------------------------------------------------------
// TXUISound (0x7C): plays a sub sound of the XUI sound bank through TXUIAudio (XuiSoundXAudio only, XACT sounds do
// nothing). The sub sound's name is the file name of the File property, lower case.
//-----------------------------------------------------------------------------
class TXUISound : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUISound, TXUIElement );

public:
	TXUISound();
	~TXUISound();

	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

private:
	TBOOL   m_bXAudio;  // 0x64 created from XuiSoundXAudio data
	TUINT32 m_uiFile;   // 0x68 string index of the File property
	TUINT32 m_eState;   // 0x6C XURXUISoundData::State
	void*   m_pSound;   // 0x70 TXUIAudio::GetSubSound (FMOD::Sound*)
	void*   m_pChannel; // 0x74 TXUIAudio::PlaySound (FMOD::Channel*)
	TFLOAT  m_fVolume;  // 0x78 linear (10 ^ (dB / 20))
};

TOSHI_NAMESPACE_END
