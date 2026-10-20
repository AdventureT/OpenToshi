#pragma once
#include <Toshi/TObject.h>
#include <Toshi/T2Vector.h>

//-----------------------------------------------------------------------------
// Purpose: the display modes (32 bits) of every output of every adapter, ARenderer::CreateTRender checks the
// -width/-height switches against them.
//-----------------------------------------------------------------------------
class ADisplayModes_Win : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ADisplayModes_Win, Toshi::TObject );

public:
	struct Mode
	{
		TUINT32 m_uiWidth;       // 0x00
		TUINT32 m_uiHeight;      // 0x04
		TUINT32 m_uiColourDepth; // 0x08
	};

public:
	// $deBlob: FUNCTION 005eb420
	ADisplayModes_Win() {}
	~ADisplayModes_Win();

	// Enumerates the modes (logs every adapter, display and mode)
	void Initialise();

	TBOOL DoesModeExist( const TUINT32& a_rWidth, const TUINT32& a_rHeight, const TUINT32& a_rColourDepth );

private:
	Toshi::T2Vector<Mode, 1024> m_Modes; // 0x04
};
