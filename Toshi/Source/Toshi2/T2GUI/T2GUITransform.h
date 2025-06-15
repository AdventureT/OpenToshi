#pragma once

TOSHI_NAMESPACE_START

class T2GUITransform
{
public:
	static constexpr TFLOAT FLOAT_QUALITY = 256;
	static constexpr TFLOAT FLOAT_SCALE   = 1 / FLOAT_QUALITY;

	using Float = TINT16;

	class Rotation
	{
	public:
		Rotation() {}
		Rotation(TINT16 x, TINT16 y)
		    : m_X(x), m_Y(y) {}

		void SetX(TFLOAT x) { m_X = PackFloat(x); }

		void SetY(TFLOAT y) { m_Y = PackFloat(y); }

		TFLOAT GetX() const { return UnpackFloat(m_X); }

		TFLOAT GetY() const { return UnpackFloat(m_Y); }

	private:
		Float m_X;
		Float m_Y;
	};

public:
	T2GUITransform()
	    : m_Rot{ { PackFloat(1.0f), 0 }, { 0, PackFloat(1.0f) } } {}

	T2GUITransform(TFLOAT basisVec1Angle, TFLOAT basisVec2Angle)
	    : m_Rot{ { PackFloat(basisVec1Angle), 0 }, { 0, PackFloat(basisVec2Angle) } } {}

	void Reset()
	{
		m_Rot[0] = { PackFloat(1.0f), 0 };
		m_Rot[1] = { 0, PackFloat(1.0f) };
	}

	void Rotate(TFLOAT angle);
	void GetInverse(T2GUITransform& outTransform);
	void PreMultiply(const T2GUITransform& transform);
	void PreMultiply(TFLOAT basisVec1Angle, TFLOAT basisVec2Angle) { PreMultiply({ basisVec1Angle, basisVec2Angle }); }
	void PostMultiply(const T2GUITransform& transform);
	void PostMultiply(TFLOAT basisVec1Angle, TFLOAT basisVec2Angle) { PostMultiply({ basisVec1Angle, basisVec2Angle }); }
	void Matrix44(TMatrix44& outMatrix);

	void Transform(TVector2& outVec, const TVector2& transformVec) const
	{
		outVec.x = m_Pos.x + m_Rot[0].GetX() * transformVec.x + m_Rot[1].GetX() * transformVec.y;
		outVec.y = m_Pos.y + m_Rot[0].GetY() * transformVec.x + m_Rot[1].GetY() * transformVec.y;
	}

	TVector2& GetPos() { return m_Pos; }

	static constexpr TFLOAT UnpackFloat(Float value) { return value * FLOAT_SCALE; }

	static constexpr Float PackFloat(TFLOAT value) { return static_cast<Float>(value * FLOAT_QUALITY); }

	static void Multiply(T2GUITransform& outTransform, const T2GUITransform& a, const T2GUITransform& b);

public:
	Rotation m_Rot[2];
	TVector2 m_Pos;
};

TOSHI_NAMESPACE_END
