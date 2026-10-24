#pragma once
#include <Math/TVector4.h>
#include <Render/TOrderTable.h>

namespace Toshi
{
class TSysMesh;
class TSysMaterial;
class TTexture;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: a sun with a lens flare: the sun (sun.tga) is a screen space quad at the projection of a fixed direction,
// the flare (corona.tga) a ring of 10 segments drawn in an orthographic projection, both fading in and out when the sun
// goes on or off screen. Never created in retail.
//-----------------------------------------------------------------------------
class ASunFlare
{
public:
	ASunFlare();
	~ASunFlare();

	void Update( TFLOAT a_fDeltaTime );
	void Render();

	void SetMagnitude( TFLOAT a_fMag );

private:
	void BuildSunMesh( const Toshi::TVector4* a_pScreenPos, TFLOAT a_fMag, TINT a_iMode );
	void BuildCoronaMesh( const Toshi::TVector4* a_pScreenPos, TFLOAT a_fMag );

	static TBOOL __stdcall ProjectToScreen( Toshi::TVector4* a_pOutScreenPos, const Toshi::TVector4* a_pDir, TBOOL* a_pOutInFront, TINT a_iUnused, TFLOAT a_fWidth, TFLOAT a_fHeight );
	static void __stdcall CreateMaterial( Toshi::TSysMaterial** a_ppMaterial, Toshi::TTexture** a_ppTexture );
	static void __stdcall CreateMesh( Toshi::TSysMesh** a_ppMesh, Toshi::TSysMaterial* a_pMaterial );
	static void __stdcall PreFlushCallback( Toshi::TOrderTable* a_pOrderTable, void* a_pCustomData );
	static void __stdcall PostFlushCallback( Toshi::TOrderTable* a_pOrderTable, void* a_pCustomData );

private:
	Toshi::TVector4      m_vDir;            // 0x00 towards the sun
	Toshi::TVector4      m_vUnk10;          // 0x10 m_vDir
	Toshi::TVector4      m_vUnk20;          // 0x20 m_vDir
	Toshi::TSysMesh*     m_pSunMesh;        // 0x30
	Toshi::TSysMaterial* m_pSunMaterial;    // 0x34
	Toshi::TSysMesh*     m_pCoronaMesh;     // 0x38
	Toshi::TSysMaterial* m_pCoronaMaterial; // 0x3C
	TUINT32              m_uiUnk40;         // 0x40
	TFLOAT               m_fMagnitude;      // 0x44
	TFLOAT               m_fSunSizeX;       // 0x48 of the display's width, at the centre of the screen
	TFLOAT               m_fSunSizeY;       // 0x4C of the display's height, at the edges
	TFLOAT               m_fFadeTime;       // 0x50
	TFLOAT               m_fFadeRate;       // 0x54
	TFLOAT               m_fMaxInFrontTime; // 0x58
	TFLOAT               m_fUnk5C;          // 0x5C
	TFLOAT               m_fFadeTimer;      // 0x60
	TFLOAT               m_fInFrontTime;    // 0x64 (never initialised)
	TBOOL                m_bVisible;        // 0x68
	TBOOL                m_bInFront;        // 0x69
	TUINT32              m_uiUnk6C;         // 0x6C
	TBOOL                m_bEnabled;        // 0x70
};
