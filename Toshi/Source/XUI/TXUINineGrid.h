#pragma once
#include "TXUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIMaterial;

//-----------------------------------------------------------------------------
// XuiNineGrid (0x48 bytes)
//-----------------------------------------------------------------------------
class XURXUINineGridData : public XURXUIElementData
{
public:
	enum Prop
	{
		Prop_TextureFileName,
		Prop_LeftOffset,
		Prop_TopOffset,
		Prop_RightOffset,
		Prop_BottomOffset,
		Prop_NoCenter,
		Prop_NUMOF,
	};

public:
	XURXUINineGridData();

	// $deBlob: FUNCTION 006e1750
	// $deBlob: FUNCTION 006e16e0
	~XURXUINineGridData() = default;

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
	TUINT16 m_uiTextureFileName; // 0x30 string index
	TUINT32 m_uiLeftOffset;      // 0x34 pixels
	TUINT32 m_uiTopOffset;       // 0x38 pixels
	TUINT32 m_uiRightOffset;     // 0x3C pixels
	TUINT32 m_uiBottomOffset;    // 0x40 pixels
	TBOOL   m_bNoCenter;         // 0x44
};

//-----------------------------------------------------------------------------
// TXUINineGrid (0x10C): a texture drawn as 9 rectangles, the borders keep their size
//-----------------------------------------------------------------------------
class TXUINineGrid : public TXUIElement
{
public:
	TDECLARE_CLASS( TXUINineGrid, TXUIElement );

public:
	// The UVs of a rectangle of the grid (0x10 bytes)
	struct Cell
	{
		TVector2 vUV1; // 0x0
		TVector2 vUV2; // 0x8
	};

	// The cells, row by row
	enum CellIndex
	{
		Cell_TopLeft,
		Cell_Top,
		Cell_TopRight,
		Cell_Left,
		Cell_Center,
		Cell_Right,
		Cell_BottomLeft,
		Cell_Bottom,
		Cell_BottomRight,
		Cell_NUMOF,
	};

public:
	TXUINineGrid();
	~TXUINineGrid();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override; // +0x14

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TUINT32 a_uiValue1, TUINT32 a_uiValue2, TFLOAT a_fScalar ) override; // +0x7C
	virtual void  UpdateTimelineProp( TXUIResource* a_pResource, TUINT32 a_uiObjectIndex, TUINT32 a_uiPropIndex, TFLOAT a_fValue1, TFLOAT a_fValue2, TFLOAT a_fScalar ) override;     // +0x80

private:
	Cell           m_aCells[ Cell_NUMOF ]; // 0x64
	TFLOAT         m_fLeftOffset;          // 0xF4
	TFLOAT         m_fTopOffset;           // 0xF8
	TFLOAT         m_fRightOffset;         // 0xFC
	TFLOAT         m_fBottomOffset;        // 0x100
	T2GUIMaterial* m_pMaterial;            // 0x104
	TBOOL          m_bNoCenter;            // 0x108
};

TOSHI_NAMESPACE_END
