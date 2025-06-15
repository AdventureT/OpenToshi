#include "ToshiPCH.h"
#include "TTransformObject.h"

TOSHI_NAMESPACE_START

TTransformObject::TTransformObject()
{
	m_EulerOrder[0] = 2;
	m_EulerOrder[1] = 1;
	m_EulerOrder[2] = 0;

	m_bChanged = TTRUE;
	m_eMode    = Mode::Matrix;
	m_Matrix.Identity();

	m_Translation = m_Matrix.GetTranslation3();
	m_ScaleVector = { 1.0f, 1.0f, 1.0f };
}

void TTransformObject::GetLocalMatrixImp(TMatrix44& outMatrix)
{
	if (m_eMode == Mode::Quat)
	{
		outMatrix.SetFromQuaternion(m_Quat);
		outMatrix.SetTranslation(m_Translation);
	}
	else if (m_eMode == Mode::Euler)
	{
		outMatrix.Identity();
		outMatrix.SetTranslation(m_Translation);

		for (TINT i = 0; i < 3; i++)
		{
			switch (m_EulerOrder[i])
			{
				case 0: RotateX(m_Euler.x); break;
				case 1: RotateX(m_Euler.y); break;
				case 2: RotateX(m_Euler.z); break;
			}
		}
	}
	else
	{
		TASSERT(m_eMode == Mode::Matrix);
		outMatrix = m_Matrix;
	}

	m_Matrix.Scale(m_ScaleVector.x, m_ScaleVector.y, m_ScaleVector.z);
}

void TTransformObject::SetEulerOrder(TUINT8 x, TUINT8 y, TUINT8 z)
{
	m_EulerOrder[0] = x;
	m_EulerOrder[1] = y;
	m_EulerOrder[2] = z;
}

void TTransformObject::SetTranslate(const TVector3& translation)
{
	m_Translation = translation;

	if (m_eMode == Mode::Matrix) m_Matrix.SetTranslation(m_Translation);

	m_bChanged = TTRUE;
}

void TTransformObject::SetQuat(const TQuaternion& quaternion)
{
	m_eMode    = Mode::Quat;
	m_bChanged = TTRUE;
	m_Quat     = quaternion;
}

void TTransformObject::SetMatrix(const TMatrix44& matrix)
{
	m_eMode       = Mode::Matrix;
	m_bChanged    = TTRUE;
	m_Matrix      = matrix;
	m_Translation = matrix.GetTranslation3();
}

TOSHI_NAMESPACE_END
