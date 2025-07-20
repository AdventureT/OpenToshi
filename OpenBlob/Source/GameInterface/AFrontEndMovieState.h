#pragma once
#include "GameInterface/AGameState.h"

#include <Toshi2/T2GUI/T2GUIRectangle.h>
#include <A2GUI/A2GUIBackground.h>
#include "Input/AInputHelper.h"

class AFrontEndMovieState : public Toshi::TGenericClassDerived<AFrontEndMovieState, AGameState, "AFrontEndMovieState", TMAKEVERSION(1, 0), TTRUE>
{
public:
	constexpr static const TCHAR* s_Assets[] = {
		"bkg_Legal_Blob_ENG",
		"THQNORDIC_LOGO_Color_white",
		"logoMovie",
		"00_ChromaCity"
	};

	typedef TUINT32 Asset;
	enum Asset_ : Asset
	{
		Asset_Legal,
		Asset_THQLogo,
		Asset_LogoMovie,
		Asset_Intro
	};

public:
	AFrontEndMovieState()
	{
		m_iAssetId            = Asset_Legal;
		m_fBackgroundLeftTime = 5.0f;
	}


	virtual UpdateResult OnUpdate(TFLOAT deltaTime) override;
	virtual void         OnInsertion() override;
	virtual void         OnRemoval() override;
	virtual void         OnActivate() override;
	virtual void         OnDeactivate() override;

#ifdef TOSHI_DEBUG
	virtual void DEBUG_RenderImGui() override;
#endif

private:
	TBOOL HasBackgroundStoppedShowing();
	TBOOL HasMovieStopped();
	void  StopMovieIfPlaying();

	void StartMovie(Asset assetId);
	void StartBlobFrontendMenu();

private:
	A2GUIBackground m_Background;          // 0x02C
	TFLOAT          m_fBackgroundLeftTime; // 0x104
	Asset           m_iAssetId;            // 0x108
	TBOOL           m_bSkip;               // 0x10C
	AInputHelper    m_InputHelper;         // 0x110
};
