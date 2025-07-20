#pragma once
#include "T2GUIElement.h"
#include "Toshi/Render/TRender.h"

TOSHI_NAMESPACE_START

class T2GUIContext
{
public:
	T2GUIContext()
	{
		auto pDisplayParams = TRender::GetSingletonSafe()->GetCurrentDisplayParams();

		m_RootElement.SetDimensions((TFLOAT)pDisplayParams->Width, (TFLOAT)pDisplayParams->Height);
		m_RootElement.SetTransform(0, 0, 0);
	}

	void Render()
	{
		m_RootElement.PreRender();
		m_RootElement.Render();
		m_RootElement.PostRender();
	}

	void Tick(TFLOAT deltaTime) { m_RootElement.Tick(deltaTime); }

	T2GUIElement* GetRootElement() { return &m_RootElement; }

private:
	T2GUIElement m_RootElement;
};

TOSHI_NAMESPACE_END
