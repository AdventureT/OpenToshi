#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUIPolygon.h"
#include "T2GUIElement.h"
#include "Math/TVector2.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f9630
// $Barnyard: FUNCTION 006d8730
T2GUIPolygon::T2GUIPolygon()
{
	m_pMaterial    = TNULL;
	m_iNumVertices = 0;
	m_pVertices    = TNULL;
	m_pUVs         = TNULL;
	m_bBoundsValid = TFALSE;
}

// $deBlob: FUNCTION 006f9670
// $deBlob: FUNCTION 006f9890
// $Barnyard: FUNCTION 006d8760
// $Barnyard: FUNCTION 006d88a0
T2GUIPolygon::~T2GUIPolygon()
{
	delete[] m_pUVs;
	delete[] m_pVertices;
}

// $deBlob: FUNCTION 006f9840
// $Barnyard: FUNCTION 006d8850
void T2GUIPolygon::Render()
{
	T2GUIRenderer* pRenderer = T2GUI::GetSingleton()->GetRenderer();

	pRenderer->SetMaterial( m_pMaterial );
	pRenderer->SetColour( m_uiColour );
	pRenderer->RenderTriStrip( m_pVertices, m_pUVs, m_iNumVertices, 1.0f, 1.0f );

	T2GUIElement::Render();
}

// $deBlob: FUNCTION 006f9730
// $Barnyard: FUNCTION 006d8790
void T2GUIPolygon::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	if ( m_bBoundsValid )
	{
		a_rWidth  = UnpackFloat( m_iWidth );
		a_rHeight = UnpackFloat( m_iHeight );
		return;
	}

	TFLOAT fMinX = TMath::MAXFLOAT;
	TFLOAT fMaxX = TMath::MINFLOAT;
	TFLOAT fMinY = TMath::MAXFLOAT;
	TFLOAT fMaxY = TMath::MINFLOAT;

	const TVector2* pVert = m_pVertices;
	for ( TINT i = m_iNumVertices; i > 0; i--, pVert++ )
	{
		if ( fMinX > pVert->x ) fMinX = pVert->x;
		if ( pVert->x > fMaxX ) fMaxX = pVert->x;
		if ( fMinY > pVert->y ) fMinY = pVert->y;
		if ( pVert->y > fMaxY ) fMaxY = pVert->y;
	}

	// NOTE: the unpacked values returned the first time are not quantized, the following calls return the packed ones
	a_rWidth       = fMaxX - fMinX;
	a_rHeight      = fMaxY - fMinY;
	m_iWidth       = PackFloat( a_rWidth );
	m_iHeight      = PackFloat( a_rHeight );
	m_bBoundsValid = TTRUE;
}

// $deBlob: FUNCTION 006f96a0
// $Barnyard: FUNCTION 006d88e0
void T2GUIPolygon::CreateBuffers( TINT a_iNumPoints )
{
	TASSERT( a_iNumPoints > 2 );

	m_iNumVertices = a_iNumPoints;
	m_pVertices    = new TVector2[ a_iNumPoints ];
	m_pUVs         = new TVector2[ a_iNumPoints ];
}

// $deBlob: FUNCTION 006f9810
void T2GUIPolygon::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	TASSERT( 0 && "T2GUIPolygon::SetDimensions(): Can't set T2GUIPolygon dimensions directly!" );
}

// $deBlob: FUNCTION 006f9720
TBOOL T2GUIPolygon::IsPointInside( const TVector2& a_rPoint )
{
	return TFALSE;
}

void T2GUIPolygon::InvalidateBounds()
{
	m_bBoundsValid = TFALSE;
}

TOSHI_NAMESPACE_END
