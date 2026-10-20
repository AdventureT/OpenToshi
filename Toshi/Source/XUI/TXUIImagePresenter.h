#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIMaterial;
class TTexture;

//-----------------------------------------------------------------------------
// XuiImagePresenter (0x3C bytes): the image of a control's visual
//-----------------------------------------------------------------------------
class XURXUIImagePresenterData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_SizeMode,
		Prop_DataAssociation,
		Prop_BrushFlags,
		Prop_NUMOF,
	};

public:
	XURXUIImagePresenterData();

	// $deBlob: FUNCTION 006dfb80
	// $deBlob: FUNCTION 006dfb10
	~XURXUIImagePresenterData() = default;

	virtual TBOOL               Load( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                                // +0x04
	virtual TUINT32             CalcPileSize( TXUIResource& a_rResource, TUINT8*& a_rData ) override;                        // +0x08
	virtual TBOOL               ValidateTimelineProp( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;             // +0x0C
	virtual TBOOL               TranslateTimelineProp( const TCHAR* a_szName, TUINT32& a_rObjectIndex, TUINT32& a_rPropIndex ) override; // +0x10
	virtual TUINT32             GetTimelinePropSize( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;              // +0x14
	virtual TBOOL               IsFloatPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                  // +0x18
	virtual TBOOL               IsColourPropType( TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex ) override;                 // +0x1C
	virtual const TXUITypeInfo* GetTypeInfo() const override;                                                                // +0x20

	TBOOL LoadData( TXUIResource& a_rResource, TUINT8*& a_rData );

	static const TXUITypeInfo* GetTypeInfoStatic() { return &sm_sTypeInfo; }

public:
	static const TXUITypeInfo sm_sTypeInfo;

public:
	TUINT32 m_uiDataAssociation; // 0x30 index of the control's image it presents
	TUINT32 m_uiSizeMode;        // 0x34 see TXUIImage::SizeMode
	TUINT32 m_uiBrushFlags;      // 0x38
};

//-----------------------------------------------------------------------------
// TXUIImagePresenter (0x8C): draws the image a TXUIVisual/TXUIGroup gives it (SetPresenterData/SetPresenterImagePath)
//-----------------------------------------------------------------------------
class TXUIImagePresenter : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUIImagePresenter, TXUIElement );

public:
	TXUIImagePresenter();
	~TXUIImagePresenter();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void Render() override;                    // +0x14

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	void SetImage( const TWCHAR* a_wszImagePath );

	// Computes the UVs from the size mode, the texture and the element's size (see TXUIImage::Init)
	void Init();

	TUINT32 GetDataAssociation() const { return m_uiDataAssociation; }

private:
	TUINT32             m_eSizeMode;         // 0x64 see TXUIImage::SizeMode
	T2GUIMaterial*      m_pMaterial;         // 0x68 created by the first SetImage
	TTexture*           m_pTexture;          // 0x6C
	TVector2            m_vUV1;              // 0x70
	TVector2            m_vUV2;              // 0x78
	TUINT32             m_uiDataAssociation; // 0x80
	TUINT32             m_uiBrushFlags;      // 0x84
	TXUIFixedVector2<6> m_vCurrentSize;      // 0x88 the element size Init has computed the UVs for
};

TOSHI_NAMESPACE_END
