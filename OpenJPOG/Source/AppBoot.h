#pragma once
#include "Toshi/Core/TApplication.h"
#include "Toshi/Core/TScheduler.h"

class AApplication : public Toshi::TApplication
{
public:

	virtual bool OnCreate(int argc, char** argv) override;
	virtual bool OnUpdate(float deltaTime) override;
	bool UpdateSound(float deltaTime);

	bool ShouldLoadModelViewState() const
	{
		return m_bLoadModelViewState;
	}

	void SetRenderWorld(bool render)
	{
		m_bRenderWorld = render;
	}

	bool ShouldRenderWorld() const
	{
		return m_bRenderWorld;
	}

public:
	static AApplication g_oTheApp;

private:
	Toshi::TTask* m_pRootTask;                       // 0x1C
	bool m_bRenderWorld;                          // 0x48
	bool m_bLoadModelViewState;                   // 0x63

};