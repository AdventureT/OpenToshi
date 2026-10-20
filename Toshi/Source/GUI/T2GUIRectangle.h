#pragma once
#include "T2GUIElement.h"
#include "T2GUIMaterial.h"
#include "T2GUITextureSection.h"

#include <Math/TVector4.h>

TOSHI_NAMESPACE_START

class T2GUIRectangle : public T2GUIElement
{
public:
	T2GUIRectangle();

	// $deBlob: FUNCTION 006f7200
	// $deBlob: FUNCTION 006f7540
	// $deBlob: FUNCTION 0076a590
	~T2GUIRectangle() {}

	virtual void Render() override; // +0x14

	/**
	 * Initialises rectangle with it's width and height.
	 */
	void Create( TFLOAT a_fWidth, TFLOAT a_fHeight );

	// Sets the material and the default UVs (the whole texture)
	void SetMaterial( T2GUIMaterial* a_pMaterial );

	// Sets the material and the texture coordinates of the four corners (only the first and the third are used by Render)
	void SetMaterial( T2GUIMaterial* a_pMaterial, const TVector2& a_rcUV1, const TVector2& a_rcUV2, const TVector2& a_rcUV3, const TVector2& a_rcUV4 );

	// Sets material and UVs from a texture section and optionally the size of the section.
	void SetMaterial( const T2GUITextureSection* a_pTexSection, TBOOL a_bSetDimensions );

	// Sets the texture coordinates of the four corners (only the first and the third are used by Render)
	void SetUVs( const TVector2& a_rcUV1, const TVector2& a_rcUV2, const TVector2& a_rcUV3, const TVector2& a_rcUV4 );
	void GetUVs( TVector2& a_rUV1, TVector2& a_rUV2, TVector2& a_rUV3, TVector2& a_rUV4 ) const;

	// Mirrors the texture horizontally (swaps the U coordinates of both pairs)
	void FlipU();

	// Mirrors the texture vertically (swaps the V coordinates of the two pairs)
	void FlipV();

	// $deBlob: FUNCTION 006f73d0
	T2GUIMaterial* GetMaterial() const { return m_pMaterial; }

protected:
	T2GUIMaterial* m_pMaterial; // 0x3C
	TVector4       m_UV1;       // 0x40
	TVector4       m_UV2;       // 0x50
};

TOSHI_NAMESPACE_END
