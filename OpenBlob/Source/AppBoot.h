#pragma once
#include "Toshi/Core/TApplication.h"
#include "GameInterface/AGameStateController.h"
#include "Render/ARenderer.h"

class AApplication : public Toshi::TApplication
{
public:
	AApplication()
	    : m_Renderer(TNULL), m_bRenderWorld(TTRUE), m_bUnk3(TFALSE)
	{
		m_szLevelPropertiesFileName = scm_sDefaultLevelPropertiesFileName;
	}
	~AApplication() {}

	virtual TBOOL OnCreate(TINT argc, TCHAR** argv) override;
	virtual TBOOL OnUpdate(TFLOAT deltaTime) override;
	TBOOL         UpdateSound(TFLOAT deltaTime);

	TBOOL ShouldLoadModelViewState() const
	{
		return m_bLoadModelViewState;
	}

	void SetRenderWorld(TBOOL render)
	{
		m_bRenderWorld = render;
	}

	TBOOL ShouldRenderWorld() const
	{
		return m_bRenderWorld;
	}

	const TCHAR* GetLevelInformationFileName() const
	{
		return m_szLevelPropertiesFileName;
	}

public:
	static AApplication        g_oTheApp;
	static inline const TCHAR* scm_sDefaultLevelPropertiesFileName = "Levels";

public:
	void*                 m_pSoundMempool;             // 0x2C
	ARenderer*            m_Renderer;                  // 0x34
	AGameStateController* m_pGameStateController;      // 0x38
	TBOOL                 m_bRenderWorld;              // 0x48
	TUINT32               m_Width;                     // 0x58
	TUINT32               m_Height;                    // 0x5C
	TBOOL                 m_bUseDefaultHeightWidth;    // 0x60
	TBOOL                 m_bIsFullscreen;             // 0x61
	TBOOL                 m_bUnk2;                     // 0x60
	TBOOL                 m_bLoadModelViewState;       // 0x63
	TBOOL                 m_bUnk3;                     // 0x64
	TBOOL                 m_bUnk4;                     // 0x65
	const TCHAR*          m_szLevelPropertiesFileName; // 0x68
};
