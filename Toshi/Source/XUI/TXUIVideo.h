#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

class TXUIMovie;

class XURXUIVideoData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_File,
		Prop_SizeMode,
		Prop_Pause,
		Prop_Loop,
		Prop_Volume,
		Prop_NUMOF,
	};

public:
	XURXUIVideoData();

	// $deBlob: FUNCTION 006bc7c0
	// $deBlob: FUNCTION 006bc730
	// $deBlob: FUNCTION 006bc790
	// $deBlob: FUNCTION 006bc720
	~XURXUIVideoData() = default;

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
	TUINT32 m_uiSizeMode; // 0x30
	TFLOAT  m_fVolume;    // 0x34 dB, -12 by default
	TUINT16 m_uiFile;     // 0x38 string index
	TBOOL   m_bPause;     // 0x3A
	TBOOL   m_bLoop;      // 0x3B
};

//-----------------------------------------------------------------------------
// TXUIVideo (0x74): plays a movie through the game's TXUIMovie (TXUI::CreateMovie)
//-----------------------------------------------------------------------------
class TXUIVideo : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUIVideo, TXUIElement );

public:
	TXUIVideo();
	~TXUIVideo();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override;      // +0x04
	virtual void Render() override;                         // +0x14
	virtual void SetVisible( TBOOL a_bVisible ) override;   // +0x48

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	// Called by the movie (AXUIBinkMovie::Update) when it reaches its end
	void OnMovieFinished();

	TBOOL      IsLooping() const { return m_bLoop; }

	// NOTE: inlined (ABlobFrontEnd writes it directly)
	// $deBlob: FUNCTION 006310c0
	void SetLooping( TBOOL a_bLoop ) { m_bLoop = a_bLoop; }
	// $deBlob: FUNCTION 006310a0
	TXUIMovie* GetMovie() const { return m_pMovie; }

private:
	TBOOL      m_bPause;  // 0x64
	TBOOL      m_bHidden; // 0x65 the movie was paused because the element or a parent is hidden
	TBOOL      m_bLoop;   // 0x66 read by AXUIBinkMovie
	TFLOAT     m_fVolume; // 0x68 linear (10 ^ (dB / 20))
	TXUIMovie* m_pMovie;  // 0x6C
	TUINT32    m_uiFile;  // 0x70 string index of the opened file
};

TOSHI_NAMESPACE_END
