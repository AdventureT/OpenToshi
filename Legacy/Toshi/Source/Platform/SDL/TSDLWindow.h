#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/Strings/TString8.h"

#include <SDL/SDL.h>

TOSHI_NAMESPACE_START

class TSDLWindowListener
{
public:
	virtual TBOOL OnEvent(const SDL_Event& a_rEvent) = 0;
};

class TSDLWindow : public TGenericClassDerived<TSDLWindow, TObject, "TSDLWindow", TMAKEVERSION(1, 0), TFALSE>
{
public:
	TSDLWindow() = default;

	TBOOL Create(TRender* a_pRender, const TCHAR* a_szTitle);

	void Update();

	void SetFullscreen(TBOOL a_bFullScreen);
	void SetPosition(TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight);

	void SetListener(TSDLWindowListener* a_pListener) { m_pListener = a_pListener; }

	SDL_Window* GetNativeWindow() const { return m_pWindow; }

private:
	SDL_Window*         m_pWindow = TNULL;
	TSDLWindowListener* m_pListener;
	TRender*            m_pRender;
};

TOSHI_NAMESPACE_END
