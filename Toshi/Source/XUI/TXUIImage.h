#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIMaterial;
class TTexture;

//-----------------------------------------------------------------------------
// XuiImage (0x3C bytes)
//-----------------------------------------------------------------------------
class XURXUIImageData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_SizeMode,
		Prop_ImagePath,
		Prop_BrushFlags,
		Prop_NUMOF,
	};

public:
	XURXUIImageData();

	// $deBlob: FUNCTION 006dea50
	// $deBlob: FUNCTION 006dd8c0
	~XURXUIImageData() = default;

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
	TUINT16 m_uiImagePath;  // 0x30 string index
	TUINT32 m_uiSizeMode;   // 0x34 see TXUIImage::SizeMode
	TUINT32 m_uiBrushFlags; // 0x38
};

//-----------------------------------------------------------------------------
// TXUIImage (0x90): a textured rectangle
//-----------------------------------------------------------------------------
class TXUIImage : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUIImage, TXUIElement );

public:
	// How the texture is mapped on the element (XUI size modes, Init)
	enum SizeMode : TUINT32
	{
		SizeMode_Tile        = 0,  // the texture at its size, repeated
		SizeMode_TextureSize = 1,  // the element takes the texture's size
		SizeMode_Centre      = 2,  // the texture at its size, centred
		SizeMode_Stretch     = 4,  // (default case) the whole texture
		SizeMode_KeepAspect  = 8,  // the whole texture, keeping its aspect ratio
		SizeMode_CentreCrop  = 16, // centred, keeping its aspect ratio
	};

public:
	TXUIImage();
	~TXUIImage();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void PreRender() override;                 // +0x10
	virtual void Render() override;                    // +0x14
	virtual void PostRender() override;                // +0x18

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

	void SetImagePath( const TWCHAR* a_wszImagePath );
	void SetUVs( const TVector2& a_rUV1, const TVector2& a_rUV2 );

	// Computes the UVs from the size mode, the texture and the element's size
	void Init();

private:
	TUINT32              m_eSizeMode;      // 0x64 see SizeMode
	T2GUIMaterial*       m_pMaterial;      // 0x68
	TBOOL                m_bOwnsMaterial;  // 0x6C TTRUE, destroys m_pMaterial in the destructor
	TVector2             m_vUV1;           // 0x70
	TVector2             m_vUV2;           // 0x78
	const TWCHAR*        m_wszImagePath;   // 0x80
	TTexture*            m_pTexture;       // 0x84
	TUINT32              m_uiBrushFlags;   // 0x88
	TXUIFixedVector2<6>  m_vCurrentSize;   // 0x8C the element size Init has computed the UVs for
};

TOSHI_NAMESPACE_END
