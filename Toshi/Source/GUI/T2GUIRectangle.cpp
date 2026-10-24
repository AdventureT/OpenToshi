#include "ToshiPCH.h"
#include "T2GUIRectangle.h"
#include "T2GUI.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f71e0
// $Barnyard: FUNCTION 006c3ae0
T2GUIRectangle::T2GUIRectangle()
{
	m_pMaterial = TNULL;
}

// $deBlob: FUNCTION 006f7210
void T2GUIRectangle::Create( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_iWidth  = PackFloat( a_fWidth );
	m_iHeight = PackFloat( a_fHeight );
}

// $deBlob: FUNCTION 006f7490
// $Barnyard: FUNCTION 006c39b0
void T2GUIRectangle::Render()
{
	T2GUIRenderer* pRenderer = T2GUI::GetSingleton()->GetRenderer();

	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );

	TFLOAT fWidth  = UnpackFloat( m_iWidth );
	TFLOAT fHeight = UnpackFloat( m_iHeight );

	TVector2 pos1 = { fWidth * -0.5f, fHeight * -0.5f };
	TVector2 pos2 = { fWidth + fWidth * -0.5f, fHeight * -0.5f + fHeight };
	pRenderer->RenderRectangle( pos1, pos2, m_UV1.AsVector2(), m_UV2.AsVector2() );

	T2GUIElement::Render();
}

// $deBlob: FUNCTION 006f7340
// $Barnyard: FUNCTION 006c3910
// $Barnyard: FUNCTION 006c3b00
void T2GUIRectangle::SetMaterial( const T2GUITextureSection* a_pTexSection, TBOOL a_bSetDimensions )
{
	m_pMaterial = a_pTexSection->GetMaterial();
	m_UV1       = a_pTexSection->GetUVPoint1();
	m_UV2       = a_pTexSection->GetUVPoint2();

	if ( a_bSetDimensions )
	{
		// NOTE: retail doesn't take the absolute value
		SetDimensions( TFLOAT( a_pTexSection->GetPoint2X() - a_pTexSection->GetPoint1X() ), TFLOAT( a_pTexSection->GetPoint2Y() - a_pTexSection->GetPoint1Y() ) );
	}
}

// $deBlob: FUNCTION 006f72f0
// $Barnyard: FUNCTION 006c38d0
void T2GUIRectangle::SetUVs( const TVector2& a_rcUV1, const TVector2& a_rcUV2, const TVector2& a_rcUV3, const TVector2& a_rcUV4 )
{
	m_UV1.x = a_rcUV1.x;
	m_UV1.y = a_rcUV1.y;
	m_UV1.z = a_rcUV2.x;
	m_UV1.w = a_rcUV2.y;
	m_UV2.x = a_rcUV3.x;
	m_UV2.y = a_rcUV3.y;
	m_UV2.z = a_rcUV4.x;
	m_UV2.w = a_rcUV4.y;
}

// $deBlob: FUNCTION 006f7250
void T2GUIRectangle::SetMaterial( T2GUIMaterial* a_pMaterial )
{
	m_UV1.Set( 0.0f, 0.0f, 1.0f, 0.0f );
	m_UV2.Set( 1.0f, 1.0f, 0.0f, 1.0f );
	m_pMaterial = a_pMaterial;
}

// $deBlob: FUNCTION 006f72a0
void T2GUIRectangle::SetMaterial( T2GUIMaterial* a_pMaterial, const TVector2& a_rcUV1, const TVector2& a_rcUV2, const TVector2& a_rcUV3, const TVector2& a_rcUV4 )
{
	m_pMaterial = a_pMaterial;
	SetUVs( a_rcUV1, a_rcUV2, a_rcUV3, a_rcUV4 );
}

// $deBlob: FUNCTION 006f7430
// $Barnyard: FUNCTION 006c3a50
void T2GUIRectangle::FlipU()
{
	// NOTE: de Blob only swaps the U coordinates (Barnyard swaps the whole UV pairs)
	TFLOAT fU1 = m_UV1.x;
	m_UV1.x    = m_UV1.z;
	m_UV1.z    = fU1;

	TFLOAT fU3 = m_UV2.x;
	m_UV2.x    = m_UV2.z;
	m_UV2.z    = fU3;
}

// $deBlob: FUNCTION 006f7460
// $Barnyard: FUNCTION 006c3a90
void T2GUIRectangle::FlipV()
{
	// NOTE: de Blob only swaps the V coordinates (Barnyard swaps the whole UV pairs)
	TFLOAT fV1 = m_UV1.y;
	m_UV1.y    = m_UV2.y;
	m_UV2.y    = fV1;

	TFLOAT fV2 = m_UV1.w;
	m_UV1.w    = m_UV2.w;
	m_UV2.w    = fV2;
}

// $deBlob: FUNCTION 006f73e0
// $Barnyard: FUNCTION 006c3970
void T2GUIRectangle::GetUVs( TVector2& a_rUV1, TVector2& a_rUV2, TVector2& a_rUV3, TVector2& a_rUV4 ) const
{
	a_rUV1.x = m_UV1.x;
	a_rUV1.y = m_UV1.y;
	a_rUV2.x = m_UV1.z;
	a_rUV2.y = m_UV1.w;
	a_rUV3.x = m_UV2.x;
	a_rUV3.y = m_UV2.y;
	a_rUV4.x = m_UV2.z;
	a_rUV4.y = m_UV2.w;
}

TOSHI_NAMESPACE_END
