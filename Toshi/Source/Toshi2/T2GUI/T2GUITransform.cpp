#include "ToshiPCH.h"
#include "T2GUITransform.h"

TOSHI_NAMESPACE_START

void T2GUITransform::Rotate(TFLOAT angle)
{
	TFLOAT fCos;
	TFLOAT fSin;
	TMath::SinCos(angle, fSin, fCos);

	TFLOAT f0X = m_Rot[0].GetX();
	TFLOAT f0Y = m_Rot[0].GetY();

	m_Rot[0].SetX(m_Rot[0].GetX() * fCos + m_Rot[1].GetX() * fSin);
	m_Rot[0].SetY(m_Rot[0].GetY() * fCos + m_Rot[1].GetY() * fSin);
	m_Rot[1].SetX(-f0X * fSin + m_Rot[1].GetX() * fCos);
	m_Rot[1].SetY(-f0Y * fSin + m_Rot[1].GetY() * fCos);
}

void T2GUITransform::GetInverse(T2GUITransform& outTransform)
{
	TASSERT(m_Rot[0].GetX() * m_Rot[1].GetY() - m_Rot[1].GetX() * m_Rot[0].GetY() != 0.0f);

	TFLOAT fVal1 = 1.0f / (m_Rot[0].GetX() * m_Rot[1].GetY() - m_Rot[1].GetX() * m_Rot[0].GetY());
	outTransform.m_Rot[0].SetX(m_Rot[1].GetY() * fVal1);
	outTransform.m_Rot[0].SetY(-m_Rot[0].GetY() * fVal1);
	outTransform.m_Rot[1].SetX(-m_Rot[1].GetX() * fVal1);
	outTransform.m_Rot[1].SetY(m_Rot[0].GetX() * fVal1);

	outTransform.m_Pos.x = 0;
	outTransform.m_Pos.y = 0;
	outTransform.m_Pos.x = outTransform.m_Rot[1].GetX() * m_Pos.y + outTransform.m_Rot[0].GetX() * m_Pos.x + outTransform.m_Pos.x;
	outTransform.m_Pos.y = outTransform.m_Rot[1].GetY() * m_Pos.y + outTransform.m_Rot[0].GetY() * m_Pos.x + outTransform.m_Pos.y;
	outTransform.m_Pos.x *= -1;
	outTransform.m_Pos.y *= -1;
}

void T2GUITransform::PreMultiply(const T2GUITransform& transform)
{
	TFLOAT fVar1 = m_Rot[0].GetX();
	TFLOAT fVar2 = m_Rot[0].GetY();
	TFLOAT fVar3 = transform.m_Rot[0].GetX();
	TFLOAT fVar8 = transform.m_Rot[0].GetY();
	TFLOAT fVar5 = m_Rot[1].GetX();
	TFLOAT fVar6 = m_Rot[1].GetY();
	TFLOAT fVar7 = transform.m_Rot[1].GetY();
	TFLOAT fVar4 = transform.m_Rot[1].GetX();
	m_Rot[0].SetX(fVar8 * fVar5 + fVar3 * fVar1);
	m_Rot[0].SetY(fVar8 * fVar6 + fVar3 * fVar2);
	m_Rot[1].SetX(fVar7 * fVar5 + fVar4 * fVar1);
	m_Rot[1].SetY(fVar7 * fVar6 + fVar4 * fVar2);
}

void T2GUITransform::PostMultiply(const T2GUITransform& transform)
{
	TFLOAT fVar1 = m_Rot[0].GetX();
	TFLOAT fVar6 = m_Rot[0].GetY();
	TFLOAT fVar2 = m_Rot[1].GetX();
	TFLOAT fVar7 = m_Rot[1].GetY();
	TFLOAT fVar4 = transform.m_Rot[0].GetX();
	TFLOAT fVar5 = transform.m_Rot[0].GetY();
	TFLOAT fVar8 = transform.m_Rot[1].GetX();
	TFLOAT fVar3 = transform.m_Rot[1].GetY();
	m_Rot[0].SetX(fVar8 * fVar6 + fVar4 * fVar1);
	m_Rot[0].SetY(fVar3 * fVar6 + fVar5 * fVar1);
	m_Rot[1].SetX(fVar8 * fVar7 + fVar4 * fVar2);
	m_Rot[1].SetY(fVar3 * fVar7 + fVar5 * fVar2);
}

void T2GUITransform::Matrix44(TMatrix44& outMatrix)
{
	outMatrix = { m_Rot[0].GetX(), m_Rot[0].GetY(), 0.0f, 0.0f, m_Rot[1].GetX(), m_Rot[1].GetY(), 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, m_Pos.x, m_Pos.y, 0.0f, 1.0f };
}

void T2GUITransform::Multiply(T2GUITransform& outTransform, const T2GUITransform& a, const T2GUITransform& b)
{
	TFLOAT a1 = a.m_Rot[0].GetX();
	TFLOAT b1 = a.m_Rot[0].GetY();
	TFLOAT c1 = a.m_Rot[1].GetX();
	TFLOAT d1 = a.m_Rot[1].GetY();

	TFLOAT a2 = b.m_Rot[0].GetX();
	TFLOAT b2 = b.m_Rot[0].GetY();
	TFLOAT c2 = b.m_Rot[1].GetX();
	TFLOAT d2 = b.m_Rot[1].GetY();

	outTransform.m_Rot[0].SetX(a1 * a2 + b1 * c2);
	outTransform.m_Rot[0].SetY(a1 * b2 + b1 * d2);
	outTransform.m_Rot[1].SetX(c1 * a2 + d1 * c2);
	outTransform.m_Rot[1].SetY(c1 * b2 + d1 * d2);

	a.Transform(outTransform.m_Pos, b.m_Pos);
}

TOSHI_NAMESPACE_END
