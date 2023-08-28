#pragma once
#include <Toshi/Core/TTask.h>
#include <Toshi/Math/TRandom.h>
#include "AOptions.h"
#include "ARenderer.h"

class ARootTask :
	public Toshi::TGenericClassDerived<ARootTask, Toshi::TTask, "ARootTask", TMAKEVERSION(1, 0), TTRUE>
{
public:
	ARootTask();
	~ARootTask() = default;

public:
	virtual bool OnUpdate(float deltaTime) override;
	virtual bool OnCreate() override;

private:
	bool CreateRenderInterface();
	void CreateARenderer()
	{
		if (m_pRenderer) m_pRenderer->Create();
		else m_pRenderer = TNULL;
	}

private:
	double m_Seconds = 0;

	Toshi::TRandom m_oRandom;
	AOptions m_oOptions;         // 0x24
	ARenderer* m_pRenderer;      // 0xD4
};