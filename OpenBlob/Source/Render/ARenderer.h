#pragma once
#include "Render/AXYZViewportManager.h"

#include <Toshi/Utils/TSingleton.h>
#include <Toshi/Render/TViewport.h>
#include <Toshi/File/TTRB.h>
#include <Toshi/Core/THPTimer.h>
#include <Toshi2/T2GUI/T2GUIRectangle.h>

/**
 * Implement methods of this class in Platform/{{OS}}/ARenderer_{{OS}}.cpp
 * In case of Windows it's Platform/Windows/ARenderer_Win.cpp
 */
class ARenderer : public Toshi::TSingleton<ARenderer>
{
public:
	using t_MainScene = void(*)(TFLOAT deltaTime, void* pCameraObject);

public:
	ARenderer();
	virtual ~ARenderer();

	TBOOL CreateInterface();
	TBOOL CreateTRender();

	void Create();
	void Update(TFLOAT deltaTime);

	void SetBackgroundColour(TUINT32 r, TUINT32 g, TUINT32 b);

private:
	void RenderLoadingScreen(TFLOAT fDeltaTime);
	void RenderGUI(TBOOL allowBackgroundClear);
	void RenderImGui(TBOOL allowBackgroundClear);
	void RenderMainScene(TFLOAT deltaTime, Toshi::TViewport* pViewport, Toshi::TCameraObject* pCameraObject, ACamera* pCamera, t_MainScene mainSceneCb, TBOOL bAllowBackgroundClear);
	void CreateMainViewport();
	void CreateTRenderResources();

	void GetAppCamera();
	void InitialiseViewPort();

private:
	static constexpr TINT MAX_VIEWPORTS_PER_FRAME = 4;

	inline static Toshi::TTRB s_BootAssetsTRB;
	inline static Toshi::THPTimer s_FrameTimer;
	inline static TINT s_iCurrentCamera = 0;

private:
	Toshi::TViewport* m_pViewport;           // 0x0C
	AXYZViewportManager* m_pViewportManager; // 0x10
	TBOOL m_bRenderGUI;                      // 0x44
	TBOOL m_bIsLoadingScreen;                // 0x46
	TUINT32 m_BackgroundColor;               // 0x48
	Toshi::T2GUIRectangle* m_pRectangle;     // 0x60
	TFLOAT m_fFrameTime;                     // 0x64
};