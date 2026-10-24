#include "ToshiPCH.h"
#include "T2GUI.h"
#include "T2GUIElement.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

const TFLOAT T2GUIElement::FLOAT_QUALITY = 6.0f;
const TFLOAT T2GUIElement::FLOAT_SCALE   = 1.0f / FLOAT_QUALITY;

TUINT8 T2GUIElement::s_uiGlobalVisMask = STATE_ALL;

// $deBlob: FUNCTION 006f5fe0
// $Barnyard: FUNCTION 006c2b10
T2GUIElement::T2GUIElement()
{
	m_pParent      = TNULL;
	m_iWidth       = 0;
	m_iHeight      = 0;
	m_cbPostRender = TNULL;
	m_uiColour     = 0xFFFFFFFF;
	m_uiState      = STATE_ALL;
	m_eAnchor      = T2GUIATTACHMENT_MIDDLECENTER;
	m_ePivot       = T2GUIATTACHMENT_MIDDLECENTER;
	m_eFlags       = FLAGS_VISIBLE | FLAGS_FOCUSED;
}

// $deBlob: FUNCTION 006f60c0
void T2GUIElement::AddChildHead( T2GUIElement* a_pElement )
{
	a_pElement->Unlink();
	a_pElement->m_pParent = this;
	a_pElement->LinkAfter( &m_Children );
}

// $deBlob: FUNCTION 006f6100
void T2GUIElement::AddChildTail( T2GUIElement* a_pElement )
{
	a_pElement->Unlink();
	a_pElement->m_pParent = this;
	a_pElement->LinkBefore( &m_Children );
}

// $deBlob: FUNCTION 006f6040
// $deBlob: FUNCTION 006f6d50
T2GUIElement::~T2GUIElement()
{
	TASSERT( m_Children.m_pNext == &m_Children );
}

// $deBlob: FUNCTION 006f6140
void T2GUIElement::AddChildBefore( T2GUIElement* a_pElement, T2GUIElement* a_pNext, TBOOL a_bUnlink )
{
	if ( a_bUnlink )
	{
		a_pElement->Unlink();
		a_pElement->m_pParent = this;
	}

	TASSERT( a_pNext->m_pParent == this );

	a_pElement->m_pNext          = a_pNext;
	a_pElement->m_pPrev          = a_pNext->m_pPrev;
	a_pNext->m_pPrev->m_pNext    = a_pElement;
	a_pElement->m_pNext->m_pPrev = a_pElement;
}

// $deBlob: FUNCTION 006f61c0
void T2GUIElement::AddChildAfter( T2GUIElement* a_pElement, T2GUIElement* a_pPrevious, TBOOL a_bUnlink )
{
	if ( a_bUnlink )
	{
		a_pElement->Unlink();
		a_pElement->m_pParent = this;
	}

	TASSERT( a_pPrevious->m_pParent == this );

	a_pElement->m_pPrev          = a_pPrevious;
	a_pElement->m_pNext          = a_pPrevious->m_pNext;
	a_pPrevious->m_pNext         = a_pElement;
	a_pElement->m_pNext->m_pPrev = a_pElement;
}

// $deBlob: FUNCTION 006f6240
// $Barnyard: FUNCTION 006c22c0
void T2GUIElement::Tick( TFLOAT a_fDeltaTime )
{
	T2GUIElementNode* pNode = m_Children.m_pNext;

	if ( pNode != &m_Children )
	{
		while ( pNode != TNULL )
		{
			T2GUIElement*     pElement = TSTATICCAST( T2GUIElement, pNode );
			T2GUIElementNode* pNext    = pNode->m_pNext;

			if ( pNext == &m_Children )
				pNext = TNULL;

			if ( !pElement->IsPaused() )
				pElement->Tick( a_fDeltaTime );

			pNode = pNext;
		}
	}
}

// $deBlob: FUNCTION 005b1dd0
TBOOL T2GUIElement::IsPaused()
{
	return TFALSE;
}

// $deBlob: FUNCTION 005b1da0
TBOOL T2GUIElement::SkipRender()
{
	return !( ( m_eFlags & FLAGS_VISIBLE ) && ( m_uiState & s_uiGlobalVisMask ) );
}

// Offsets a_rX/a_rY by the anchor point in a parent of the specified size
static void T2GUIElement_AnchorPos( T2GUIATTACHMENT a_eAnchor, TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	switch ( a_eAnchor )
	{
		case T2GUIATTACHMENT_TOPLEFT:
			a_rX -= a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_TOPCENTER:
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_TOPRIGHT:
			a_rX = a_fWidth * 0.5f + a_rX;
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_MIDDLELEFT:
			a_rX -= a_fWidth * 0.5f;
			break;
		case T2GUIATTACHMENT_MIDDLERIGHT:
			a_rX = a_fWidth * 0.5f + a_rX;
			break;
		case T2GUIATTACHMENT_BOTTOMLEFT:
			a_rX -= a_fWidth * 0.5f;
			a_rY = a_fHeight * 0.5f + a_rY;
			break;
		case T2GUIATTACHMENT_BOTTOMCENTER:
			a_rY = a_fHeight * 0.5f + a_rY;
			break;
		case T2GUIATTACHMENT_BOTTOMRIGHT:
			a_rX = a_fWidth * 0.5f + a_rX;
			a_rY = a_fHeight * 0.5f + a_rY;
			break;
	}
}

// Offsets a_rX/a_rY by the pivot point of an element of the specified size
static void T2GUIElement_PivotPos( T2GUIATTACHMENT a_ePivot, TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	switch ( a_ePivot )
	{
		case T2GUIATTACHMENT_TOPLEFT:
			a_rX = a_fWidth * 0.5f + a_rX;
			a_rY = a_fHeight * 0.5f + a_rY;
			break;
		case T2GUIATTACHMENT_TOPCENTER:
			a_rY = a_fHeight * 0.5f + a_rY;
			break;
		case T2GUIATTACHMENT_TOPRIGHT:
			a_rX -= a_fWidth * 0.5f;
			a_rY = a_fHeight * 0.5f + a_rY;
			break;
		case T2GUIATTACHMENT_MIDDLELEFT:
			a_rX = a_fWidth * 0.5f + a_rX;
			break;
		case T2GUIATTACHMENT_MIDDLERIGHT:
			a_rX -= a_fWidth * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMLEFT:
			a_rX = a_fWidth * 0.5f + a_rX;
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMCENTER:
			a_rY -= a_fHeight * 0.5f;
			break;
		case T2GUIATTACHMENT_BOTTOMRIGHT:
			a_rX -= a_fWidth * 0.5f;
			a_rY -= a_fHeight * 0.5f;
			break;
	}
}

// $deBlob: FUNCTION 006f6c30
void T2GUIElement::ApplyPivot( TFLOAT& a_rX, TFLOAT& a_rY, TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	T2GUIElement_PivotPos( m_ePivot, a_rX, a_rY, a_fWidth, a_fHeight );
}

// $deBlob: FUNCTION 006f6290
// $Barnyard: FUNCTION 006c2300
void T2GUIElement::PreRender()
{
	TVector2 vecAnchor( 0.0f, 0.0f );
	TVector2 vecPivot( 0.0f, 0.0f );
	TFLOAT   fWidth, fHeight;

	if ( m_pParent != TNULL )
	{
		m_pParent->GetDimensions( fWidth, fHeight );
		T2GUIElement_AnchorPos( m_eAnchor, vecAnchor.x, vecAnchor.y, fWidth, fHeight );
	}

	GetDimensions( fWidth, fHeight );
	T2GUIElement_PivotPos( m_ePivot, vecPivot.x, vecPivot.y, fWidth, fHeight );

	if ( m_eFlags & FLAGS_XUIELEMENT )
	{
		const TVector2& rPivot = GetPivot();
		vecPivot.x             = rPivot.x * -1.0f + vecPivot.x;
		vecPivot.y             = rPivot.y * -1.0f + vecPivot.y;
	}

	T2GUI::GetSingleton()->GetRenderer()->PushTransform( m_oTransform, vecAnchor, vecPivot );
}

// $deBlob: FUNCTION 006f6540
// $Barnyard: FUNCTION 006c2510
void T2GUIElement::Render()
{
	T2GUIElementNode* pNode = m_Children.m_pNext;

	if ( pNode != &m_Children && pNode != TNULL )
	{
		do
		{
			T2GUIElement* pElement = TSTATICCAST( T2GUIElement, pNode );

			if ( !pElement->SkipRender() )
			{
				pElement->PreRender();
				pElement->Render();
				pElement->PostRender();
			}

			pNode = pNode->m_pNext;
		} while ( pNode != &m_Children && pNode != TNULL );
	}

	if ( m_cbPostRender )
		m_cbPostRender();
}

// $deBlob: FUNCTION 006f6590
// $Barnyard: FUNCTION 006c2570
void T2GUIElement::PostRender()
{
	T2GUI::GetSingleton()->GetRenderer()->PopTransform();
}

// $deBlob: FUNCTION 005b1a50
// $Barnyard: FUNCTION 006c25d0
void T2GUIElement::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	a_rWidth  = UnpackFloat( m_iWidth );
	a_rHeight = UnpackFloat( m_iHeight );
}

// $deBlob: FUNCTION 005b1b70
// $Barnyard: FUNCTION 005748f0
TFLOAT T2GUIElement::GetWidth()
{
	return UnpackFloat( m_iWidth );
}

// $deBlob: FUNCTION 005b1a90
// $Barnyard: FUNCTION 00577290
TFLOAT T2GUIElement::GetHeight()
{
	return UnpackFloat( m_iHeight );
}

// $deBlob: FUNCTION 005b1d40
// $Barnyard: FUNCTION 0043ce00
void T2GUIElement::SetWidth( TFLOAT a_fWidth )
{
	TASSERT( a_fWidth >= 0.0f );
	m_iWidth = PackFloat( a_fWidth );
}

// $deBlob: FUNCTION 005b1ca0
// $Barnyard: FUNCTION 0043ce10
void T2GUIElement::SetHeight( TFLOAT a_fHeight )
{
	TASSERT( a_fHeight >= 0.0f );
	m_iHeight = PackFloat( a_fHeight );
}

// $deBlob: FUNCTION 005b1c40
// $Barnyard: FUNCTION 006c25f0
void T2GUIElement::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_iWidth  = PackFloat( a_fWidth );
	m_iHeight = PackFloat( a_fHeight );
}

// $deBlob: FUNCTION 005b1b10
// $Barnyard: FUNCTION 006c2610
void T2GUIElement::GetMins( TFLOAT& a_rX, TFLOAT& a_rY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );

	a_rX = fWidth * -0.5f;
	a_rY = fHeight * -0.5f;
}

// $deBlob: FUNCTION 005b1ac0
// $Barnyard: FUNCTION 006c2650
void T2GUIElement::GetMaxs( TFLOAT& a_rX, TFLOAT& a_rY )
{
	TFLOAT fWidth, fHeight;
	GetDimensions( fWidth, fHeight );

	a_rX = fWidth * 0.5f;
	a_rY = fHeight * 0.5f;
}

// $deBlob: FUNCTION 005b1c30
// $Barnyard: FUNCTION 0043cdf0
void T2GUIElement::SetColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
}

// $deBlob: FUNCTION 005b1c00
// $Barnyard: FUNCTION 006c2ad0
void T2GUIElement::SetAlpha( TFLOAT a_fAlpha )
{
	m_uiColour &= 0x00FFFFFF;
	m_uiColour |= TUINT32( a_fAlpha * 255.0f ) << 24;
}

// $deBlob: FUNCTION 005b1a20
TFLOAT T2GUIElement::GetAlpha()
{
	return TFLOAT( ( m_uiColour >> 24 ) & 0xFF ) / 255.0f;
}

// $deBlob: FUNCTION 005b1d20
void T2GUIElement::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == TTRUE )
		m_eFlags |= FLAGS_VISIBLE;
	else
		m_eFlags &= ~FLAGS_VISIBLE;
}

// $deBlob: FUNCTION 005b1c80
// $Barnyard: FUNCTION 006c2a80
void T2GUIElement::SetFocus( TBOOL a_bFocused )
{
	if ( a_bFocused == TTRUE )
		m_eFlags |= FLAGS_FOCUSED;
	else
		m_eFlags &= ~FLAGS_FOCUSED;
}

// $deBlob: FUNCTION 006f6600
// $Barnyard: FUNCTION 006c2690
void T2GUIElement::GetScreenTransform( T2GUITransform& a_rOutTransform )
{
	TVector2       vecAnchor( 0.0f, 0.0f );
	TVector2       vecPivot( 0.0f, 0.0f );
	T2GUITransform oParentTransform;
	T2GUITransform oLocalTransform;
	TFLOAT         fWidth, fHeight;

	if ( m_pParent != TNULL )
	{
		m_pParent->GetDimensions( fWidth, fHeight );
		T2GUIElement_AnchorPos( m_eAnchor, vecAnchor.x, vecAnchor.y, fWidth, fHeight );
	}

	GetDimensions( fWidth, fHeight );
	T2GUIElement_PivotPos( m_ePivot, vecPivot.x, vecPivot.y, fWidth, fHeight );

	if ( m_eFlags & FLAGS_XUIELEMENT )
	{
		const TVector2& rPivot = GetPivot();
		vecPivot.x             = rPivot.x * -1.0f + vecPivot.x;
		vecPivot.y             = rPivot.y * -1.0f + vecPivot.y;
	}

	if ( m_pParent != TNULL )
		m_pParent->GetScreenTransform( oParentTransform );

	// Move the parent's origin to the anchor point
	oParentTransform.Transform( oParentTransform.m_Pos, vecAnchor );

	// Move the local origin to the pivot point
	oLocalTransform = m_oTransform;
	m_oTransform.Transform( oLocalTransform.m_Pos, vecPivot );

	if ( m_eFlags & FLAGS_XUIELEMENT )
	{
		const TVector2& rPivot = GetPivot();
		oLocalTransform.m_Pos.x += rPivot.x;
		oLocalTransform.m_Pos.y += rPivot.y;
	}

	T2GUITransform::Multiply( a_rOutTransform, oParentTransform, oLocalTransform );
}

// $deBlob: FUNCTION 006f6bc0
// $Barnyard: FUNCTION 006c2a00
TBOOL T2GUIElement::IsPointInside( const TVector2& a_rPoint )
{
	TFLOAT fMinX, fMinY;
	TFLOAT fMaxX, fMaxY;

	GetMins( fMinX, fMinY );
	GetMaxs( fMaxX, fMaxY );

	if ( fMinX <= a_rPoint.x && a_rPoint.x <= fMaxX && fMinY <= a_rPoint.y )
		return a_rPoint.y <= fMaxY;

	return TFALSE;
}

// $deBlob: FUNCTION 005b1b60
const TVector2& T2GUIElement::GetPivot()
{
	return TVector2::VEC_ZERO;
}

// $deBlob: FUNCTION 006f6b60
// $Barnyard: FUNCTION 006c29d0
void T2GUIElement::GetInvScreenTransform( T2GUITransform& a_rOutTransform )
{
	T2GUITransform oScreenTransform;
	GetScreenTransform( oScreenTransform );
	oScreenTransform.GetInverse( a_rOutTransform );
}

TOSHI_NAMESPACE_END
