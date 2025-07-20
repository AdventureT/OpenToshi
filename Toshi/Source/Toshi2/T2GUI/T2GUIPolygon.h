#pragma once

#include "T2GUI.h"

TOSHI_NAMESPACE_START

class T2GUIPolygon : public T2GUIElement
{
public:
	T2GUIPolygon()
	{
		m_iNumVerts             = 0;
		m_pMaterial             = TNULL;
		m_pVertices             = TNULL;
		m_pUV                   = TNULL;
		m_bCalculatedDimensions = TFALSE;
	}

	~T2GUIPolygon() { T2GUI::DestroyMaterial(m_pMaterial); }

	virtual void Render();
	virtual void GetDimensions(TFLOAT& width, TFLOAT& height);
	virtual void SetDimensions(TFLOAT width, TFLOAT height);

	void Create(TINT numpoints);

	void SetMaterial(T2GUIMaterial* pMaterial) { m_pMaterial = pMaterial; }

	TVector2* GetVertices() const { return m_pVertices; }

	TVector2* GetUV() const { return m_pUV; }

private:
	T2GUIMaterial* m_pMaterial;
	TUINT32        m_iNumVerts;
	TVector2*      m_pVertices;
	TVector2*      m_pUV;
	TBOOL          m_bCalculatedDimensions;
};

TOSHI_NAMESPACE_END
