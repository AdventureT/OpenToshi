#include "ToshiPCH.h"
#include "T2GUIElement.h"
#include "T2GUI.h"

TOSHI_NAMESPACE_START

constinit TUINT32 T2GUIElement::s_uiGlobalVisMask = 0xFF;

// $deBlob: FUNCTION 006f5fe0
T2GUIElement::T2GUIElement()
{
	m_pParent      = TNULL;
	m_Width        = 0;
	m_Height       = 0;
	m_Color        = 0xFFFFFFFF;
	m_cbPostRender = TNULL;
	m_State        = 255;
	m_Pivot        = Pivot::MiddleCenter;
	m_Anchor       = Anchor::MiddleCenter;
	m_Flags        = 3;
}

// $deBlob: FUNCTION 006f6040
T2GUIElement::~T2GUIElement()
{
	TASSERT(!m_Children.IsLinked());
}

// $deBlob: FUNCTION 006f6240
void T2GUIElement::Tick(TFLOAT deltaTime)
{
	for (auto it = m_Children.Front(); it != m_Children.End(); it++)
	{
		if (!it->IsPaused())
		{
			it->Tick(deltaTime);
		}
	}
}

// $deBlob: FUNCTION 005b1dd0
TBOOL T2GUIElement::IsPaused() const
{
	return TFALSE;
}

// $deBlob: FUNCTION 005b1da0
TBOOL T2GUIElement::SkipRender()
{
	if (HASFLAG(m_Flags & FLAGS_VISIBLE) && HASFLAG(m_Flags & (TUINT8)s_uiGlobalVisMask))
	{
		return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006f6290
void T2GUIElement::PreRender()
{
	TVector2 vec1 = { 0.0f, 0.0f };
	TVector2 vec2 = { 0.0f, 0.0f };

	if (m_pParent != TNULL)
	{
		TFLOAT fParentWidth, fParentHeight;
		m_pParent->GetDimensions(fParentWidth, fParentHeight);

		switch (m_Anchor)
		{
			case Anchor::BottomLeft:
				vec1.x = vec1.x - fParentWidth * 0.5f;
				vec1.y = vec1.y - fParentHeight * 0.5f;
				break;
			case Anchor::BottomCenter: vec1.y = vec1.y - fParentHeight * 0.5f; break;
			case Anchor::BottomRight:
				vec1.y = vec1.y - fParentHeight * 0.5f;
				vec1.x = fParentWidth * 0.5f + vec1.x;
				break;
			case Anchor::MiddleLeft: vec1.x = vec1.x - fParentWidth * 0.5f; break;
			case Anchor::MiddleRight: vec1.x = fParentWidth * 0.5f + vec1.x; break;
			case Anchor::TopLeft:
				vec1.x = vec1.x - fParentWidth * 0.5f;
				vec1.y = fParentHeight * 0.5f + vec1.y;
				break;
			case Anchor::TopCenter: vec1.y = fParentHeight * 0.5f + vec1.y; break;
			case Anchor::TopRight:
				vec1.x = fParentWidth * 0.5f + vec1.x;
				vec1.y = fParentHeight * 0.5f + vec1.y;
				break;
		}
	}

	TFLOAT fWidth, fHeight;
	GetDimensions(fWidth, fHeight);

	switch (m_Pivot)
	{
		case Pivot::BottomLeft:
			vec2.x = fWidth * 0.5f + vec2.x;
			vec2.y = fHeight * 0.5f + vec2.y;
			break;
		case Pivot::BottomCenter: vec2.y = fHeight * 0.5f + vec2.y; break;
		case Pivot::BottomRight:
			vec2.x = vec2.x - fWidth * 0.5f;
			vec2.y = fHeight * 0.5f + vec2.y;
			break;
		case Pivot::MiddleRight: vec2.x = vec2.x - fWidth * 0.5f; break;
		case Pivot::MiddleLeft: vec2.x = fWidth * 0.5f + vec2.x; break;
		case Pivot::TopLeft:
			vec2.x = fWidth * 0.5f + vec2.x;
			vec2.y = vec2.y - fHeight * 0.5f;
			break;
		case Pivot::TopCenter: vec2.y = vec2.y - fHeight * 0.5f; break;
		case Pivot::TopRight:
			vec2.x = vec2.x - fWidth * 0.5f;
			vec2.y = vec2.y - fHeight * 0.5f;
			break;
	}

	if (HASFLAG(m_Flags & FLAGS_XUIELEMENT))
	{
		auto pivot = GetPivot();
		vec2.x     = -pivot.x + vec2.x;
		vec2.y     = -pivot.y + vec2.y;
	}

	T2GUI::GetSingleton()->GetRenderer()->PushTransform(m_Transform, vec1, vec2);
}

// $deBlob: FUNCTION 006f6540
void T2GUIElement::Render()
{
	for (auto it = m_Children.Front(); it != m_Children.End(); it++)
	{
		if (!it->SkipRender())
		{
			it->PreRender();
			it->Render();
			it->PostRender();
		}
	}

	if (m_cbPostRender)
	{
		m_cbPostRender();
	}
}

// $deBlob: FUNCTION 006f6590
void T2GUIElement::PostRender()
{
	T2GUI::GetSingleton()->GetRenderer()->PopTransform();
}

// $deBlob: FUNCTION 005b1a50
void T2GUIElement::GetDimensions(TFLOAT& width, TFLOAT& height)
{
	width  = UnpackFloat(m_Width);
	height = UnpackFloat(m_Height);
}

// $deBlob: FUNCTION 005b1b70
TFLOAT T2GUIElement::GetWidth()
{
	return UnpackFloat(m_Width);
}

// $deBlob: FUNCTION 005b1a90
TFLOAT T2GUIElement::GetHeight()
{
	return UnpackFloat(m_Height);
}

// $deBlob: FUNCTION 005b1d40
void T2GUIElement::SetWidth(TFLOAT width)
{
	TASSERT(width >= 0.0f);
	m_Width = PackFloat(width);
}

// $deBlob: FUNCTION 005b1ca0
void T2GUIElement::SetHeight(TFLOAT height)
{
	TASSERT(height >= 0.0f);
	m_Height = PackFloat(height);
}

// $deBlob: FUNCTION 005b1c40
void T2GUIElement::SetDimensions(TFLOAT width, TFLOAT height)
{
	TASSERT(width >= 0.0f);
	TASSERT(height >= 0.0f);
	m_Width  = PackFloat(width);
	m_Height = PackFloat(height);
}

// $deBlob: FUNCTION 005b1b10
void T2GUIElement::GetMins(TFLOAT& x, TFLOAT& y)
{
	TFLOAT width, height;
	GetDimensions(width, height);
	x = width * -0.5f;
	y = height * -0.5f;
}

// $deBlob: FUNCTION 005b1ac0
void T2GUIElement::GetMaxs(TFLOAT& x, TFLOAT& y)
{
	TFLOAT width, height;
	GetDimensions(width, height);
	x = width * 0.5f;
	y = height * 0.5f;
}

// $deBlob: FUNCTION 005b1c30
void T2GUIElement::SetColour(TUINT32 colour)
{
	m_Color = colour;
}

// $deBlob: FUNCTION 005b1c00
void T2GUIElement::SetAlpha(TFLOAT alpha)
{
	TUINT8 alpha8 = (TUINT8)(alpha * 255.0f);
	m_Color       = (m_Color & 0x00FFFFFF) | alpha8 << 24;
}

// $deBlob: FUNCTION 005b1a20
TFLOAT T2GUIElement::GetAlpha()
{
	return ((m_Color & 0x00FFFFFF) >> 24) * 255.0f;
}

// $deBlob: FUNCTION 005b1d20
void T2GUIElement::SetVisible(TBOOL visible)
{
	if (visible)
	{
		m_Flags |= FLAGS_VISIBLE;
	}
	else
	{
		m_Flags &= ~FLAGS_VISIBLE;
	}
}

// $deBlob: FUNCTION 005b1c80
void T2GUIElement::SetFocus(TBOOL focused)
{
	if (focused)
	{
		m_Flags |= FLAGS_FOCUSED;
	}
	else
	{
		m_Flags &= ~FLAGS_FOCUSED;
	}
}

// $deBlob: FUNCTION 006f6600
void T2GUIElement::GetScreenTransform(T2GUITransform& outTransform)
{
	TIMPLEMENT();
}

// $deBlob: FUNCTION 006f6bc0
TBOOL T2GUIElement::IsPointInside(const TVector2& point)
{
	TFLOAT minX, minY;
	TFLOAT maxX, maxY;

	GetMins(minX, minY);
	GetMaxs(maxX, maxY);

	if (minX <= point.x && point.x <= maxX && minY <= point.y)
	{
		return point.y <= maxY;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005b1b60
const TVector2& T2GUIElement::GetPivot()
{
	return TVector2::VEC_ZERO;
}

// $deBlob: FUNCTION 006f6b60
void T2GUIElement::GetInvScreenTransform(T2GUITransform& outTransform)
{
	T2GUITransform screenTransform;
	GetScreenTransform(screenTransform);
	screenTransform.GetInverse(outTransform);
}

TOSHI_NAMESPACE_END
