#pragma once
#include "T2GUITransform.h"
#include "Toshi2/T2DList.h"

TOSHI_NAMESPACE_START

/*
	class T2GUIElementNode : public T2DList<T2GUIElementNode>::Node
	{
	public:
		virtual ~T2GUIElementNode() = default;
	};*/

class T2GUIElement : public T2DList<T2GUIElement>::Node
{
public:
	static constexpr TFLOAT FLOAT_QUALITY = 6;
	static constexpr TFLOAT FLOAT_SCALE   = 1 / FLOAT_QUALITY;

	using Float = TUINT16;

	using t_PostRender = void (*)();

	enum class Pivot : TUINT8
	{
		BottomLeft,
		BottomCenter,
		BottomRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		TopLeft,
		TopCenter,
		TopRight
	};

	enum class Anchor : TUINT8
	{
		BottomLeft,
		BottomCenter,
		BottomRight,
		MiddleLeft,
		MiddleCenter,
		MiddleRight,
		TopLeft,
		TopCenter,
		TopRight
	};

	typedef TUINT8 FLAGS;
	enum FLAGS_ : FLAGS
	{
		FLAGS_VISIBLE    = BITFIELD(0),
		FLAGS_FOCUSED    = BITFIELD(1),
		FLAGS_XUIELEMENT = BITFIELD(2),
	};

public:
	T2GUIElement();
	virtual ~T2GUIElement();

	virtual void            Tick(TFLOAT deltaTime);
	virtual TBOOL           IsPaused() const;
	virtual TBOOL           SkipRender();
	virtual void            PreRender();
	virtual void            Render();
	virtual void            PostRender();
	virtual void            GetDimensions(TFLOAT& width, TFLOAT& height);
	virtual TFLOAT          GetWidth();
	virtual TFLOAT          GetHeight();
	virtual void            SetWidth(TFLOAT width);
	virtual void            SetHeight(TFLOAT height);
	virtual void            SetDimensions(TFLOAT width, TFLOAT height);
	virtual void            GetMins(TFLOAT& x, TFLOAT& y);
	virtual void            GetMaxs(TFLOAT& x, TFLOAT& y);
	virtual void            SetColour(TUINT32 colour);
	virtual void            SetAlpha(TFLOAT alpha);
	virtual TFLOAT          GetAlpha();
	virtual void            SetVisible(TBOOL visible);
	virtual void            SetFocus(TBOOL focused);
	virtual void            GetScreenTransform(T2GUITransform& outTransform);
	virtual TBOOL           IsPointInside(const TVector2& point);
	virtual const TVector2& GetPivot();

	void GetInvScreenTransform(T2GUITransform& outTransform);

	void Unlink()
	{
		Node::Remove();
		m_pParent = TNULL;
	}

	void AddChildTail(T2GUIElement* pElement)
	{
		if (pElement->IsLinked()) pElement->Remove();
		pElement->m_pParent = this;
		m_Children.PushBack(pElement);
	}

	void AddChildHead(T2GUIElement* pElement)
	{
		if (pElement->IsLinked()) pElement->Remove();
		pElement->m_pParent = this;
		m_Children.PushFront(pElement);
	}

	void SetTransform(TFLOAT x, TFLOAT y, TFLOAT angle = 0.0f)
	{
		m_Transform.Reset();
		m_Transform.GetPos().x = 0;
		m_Transform.GetPos().y = 0;
		m_Transform.Rotate(angle);
		m_Transform.GetPos().x = x;
		m_Transform.GetPos().y = y;
	}

	void SetAnchor(Anchor pivot) { m_Anchor = pivot; }

	void SetPivot(Pivot pivot) { m_Pivot = pivot; }

	static TFLOAT UnpackFloat(Float value) { return value * FLOAT_SCALE; }

	static Float PackFloat(TFLOAT value) { return static_cast<Float>(value * FLOAT_QUALITY); }

protected:
	static constinit TUINT32 s_uiGlobalVisMask;

protected:
	T2GUIElement*         m_pParent;      // 0x0C
	T2GUITransform        m_Transform;    // 0x10
	T2DList<T2GUIElement> m_Children;     // 0x20
	Float                 m_Width;        // 0x2C
	Float                 m_Height;       // 0x2E
	TUINT32               m_Color;        // 0x30
	t_PostRender          m_cbPostRender; // 0x34
	FLAGS                 m_State;        // 0x38
	Anchor                m_Anchor;       // 0x39
	Pivot                 m_Pivot;        // 0x3A
	FLAGS                 m_Flags;        // 0x3B
};

TOSHI_NAMESPACE_END
