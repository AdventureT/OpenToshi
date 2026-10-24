#pragma once
#include "T2GUIElement.h"

TOSHI_NAMESPACE_START

class T2GUIMaterial;

class T2GUIRectangle : public T2GUIElement
{
public:
	T2GUIRectangle();

	virtual void Render() override;

	// $deBlob: FUNCTION 006f7210
	void Create(TFLOAT width, TFLOAT height)
	{
		m_Width  = PackFloat(width);
		m_Height = PackFloat(height);
	}

	void SetMaterial(T2GUIMaterial* pMaterial);

private:
	T2GUIMaterial* m_pMaterial;
	TVector2       m_UV1;
	TVector2       m_UV2;
};

TOSHI_NAMESPACE_END
