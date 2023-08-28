#pragma once

#include <Toshi/Render/TViewport.h>

class ARenderer : 
	public Toshi::TGenericClassDerived<ARenderer, Toshi::TTask, "ARenderer", TMAKEVERSION(1, 0), TTRUE>
{
public:

	class PreRenderEvent
	{

	};

	virtual TBOOL OnCreate();
	virtual TBOOL OnUpdate(float deltaTime);
	virtual void OnDestroy();


	float GetCurrentRenderTime() { return m_fCurRenderTime; }
	float GetLastRenderTime() { return m_fLastRenderTime; }
	float GetFarClip() { return m_fFarClip; }
	uint8_t* GetScreenCaptureBuffer() { return m_pcScreenCaptureBuffer; }
	Toshi::TViewport* GetViewport() { return m_pViewport; }

	Toshi::TEmitter<ARenderer, PreRenderEvent>& GetPreRenderEmitter() { return m_oPreRenderEmitter; }


private:
	float m_fLastRenderTime;                                 // 0x24
	float m_fCurRenderTime;	                                 // 0x28
	Toshi::TViewport* m_pViewport;                           // 0x2C
	float m_fFarClip;		                                 // 0x38
	uint8_t* m_pcScreenCaptureBuffer;                        // 0x44
	void* m_pWaterShader;                                    // 0x50
	Toshi::TEmitter<ARenderer, PreRenderEvent> m_oPreRenderEmitter; // 0x90
};

