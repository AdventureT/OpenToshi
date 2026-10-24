#pragma once
#include "TXUIElement.h"
#include "TXUITimelineControl.h"

TOSHI_NAMESPACE_START

class TXUIScene;

class XURXUICanvasData : public XURXUIElementData
{
public:
	XURXUICanvasData();
	// $deBlob: FUNCTION 006dd840
	~XURXUICanvasData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;
	virtual const TXUITypeInfo* GetTypeInfo() const override;

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;
};

//-----------------------------------------------------------------------------
// TXUICanvas (0xB4): the root of the scenes (TXUI's canvas) and of every .xur
//-----------------------------------------------------------------------------
class TXUICanvas : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUICanvas, TXUIElement );

public:
	TXUICanvas();
	~TXUICanvas();

	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  PlayAnimation( TINT a_iStartFrame, TINT a_iEndFrame ) override;                                       // +0x68
	virtual void  StopAnimation() override;                                                                             // +0x74

	virtual void AddScene( TXUIElement* a_pElement ); // +0xC0

private:
	TXUITimelineControl m_oTimelineControl; // 0x64
};

TOSHI_NAMESPACE_END
