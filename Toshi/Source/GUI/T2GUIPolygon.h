#pragma once
#include "T2GUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIPolygon : public T2GUIElement
{
public:
	T2GUIPolygon();
	~T2GUIPolygon();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void  Render() override;                                       // +0x14
	virtual void  GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) override; // +0x1C
	virtual void  SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) override;   // +0x30
	virtual TBOOL IsPointInside( const TVector2& a_rPoint ) override;      // +0x54

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void CreateBuffers( TINT a_iNumPoints );

	void InvalidateBounds();
	// $deBlob: FUNCTION 005d5fc0
	void SetMaterial( T2GUIMaterial* a_pMaterial ) { m_pMaterial = a_pMaterial; }
	T2GUIMaterial* GetMaterial() const { return m_pMaterial; }

	TINT            GetNumVertices() const { return m_iNumVertices; }
	TVector2*       GetVertices() { return m_pVertices; }
	const TVector2* GetVertices() const { return m_pVertices; }
	TVector2*       GetUVs() { return m_pUVs; }
	const TVector2* GetUVs() const { return m_pUVs; }

private:
	T2GUIMaterial* m_pMaterial;    // 0x3C
	TINT           m_iNumVertices; // 0x40
	TVector2*      m_pVertices;    // 0x44
	TVector2*      m_pUVs;         // 0x48
	TBOOL          m_bBoundsValid; // 0x4C
};

TOSHI_NAMESPACE_END
