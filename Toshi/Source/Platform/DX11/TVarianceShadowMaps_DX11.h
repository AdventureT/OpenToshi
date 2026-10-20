#pragma once
#include "Math/TMatrix44.h"
#include "Math/TVector4.h"
#include "Math/TVector3.h"

TOSHI_NAMESPACE_START

class TTexture;

//-----------------------------------------------------------------------------
// Created by TRenderDX11::CreateDisplay. NOTE: on DX11 most of it is stubbed out (TASSERT( TFALSE )) and only the constructor and
// destructor are called; the rest of the file isn't reconstructed yet.
//-----------------------------------------------------------------------------
class TVarianceShadowMaps
{
public:
	TVarianceShadowMaps();
	~TVarianceShadowMaps();

	void SetPosition( const TVector3& a_rcPosition );
	void SetOrthographicProjection( TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fNear, TFLOAT a_fFar );
	void SetActiveFrustum( TINT a_iActiveFrustum );

	// The DX9 leftovers: on DX11 they only assert TFALSE (the arguments aren't read) and nothing calls them
	void Unk409( void* a_pUnk );
	void Unk492();
	void Unk520();
	void Unk557();
	void Unk582( void* a_pUnk );
	void Unk608();
	void Unk641();
	void Unk653();
	void Unk787();
	void Unk800( void* a_pUnk );

	// Unk582/Unk800 when s_bUnk582/s_bUnk800 are set (both are), returns 0
	TINT Unk582And800( void* a_pUnk );

	// Uploads the transposed a_rcMatrix1 (c0-c3) and a_rcMatrix2 (c4-c7), the normalised m_vUnkC0 (c8) and m_vUnk80
	// (c9) to the vertex shader constants
	void SetShaderConstants( const TMatrix44& a_rcMatrix1, const TMatrix44& a_rcMatrix2 );

private:
	TINT Initialise();

	// Takes m_vUnkC0 from the render's parameter table (V4 parameter 2, w 0) and sets up function statics nothing reads;
	// returns 0
	TINT UpdateUnkC0();

	static TBOOL s_bUnk582;
	static TBOOL s_bUnk800;

private:
	TUINT32   m_aUnk00[ 16 ];    // 0x00
	TMatrix44 m_oProjection;     // 0x40
	TVector4  m_vUnk80;          // 0x80
	TVector4  m_vUnk90;          // 0x90
	TVector4  m_vUnkA0;          // 0xA0
	TVector4  m_vPosition;       // 0xB0
	TVector4  m_vUnkC0;          // 0xC0
	TINT      m_iNumFrustums;    // 0xD0
	TINT      m_iActiveFrustum;  // 0xD4
	TUINT32   m_uiUnkD8;         // 0xD8
	TFLOAT    m_fUnkDC;          // 0xDC
	TFLOAT    m_fUnkE0;          // 0xE0
	TFLOAT    m_fUnkE4;          // 0xE4
	TINT      m_iShadowMapSize;  // 0xE8 1024
	TUINT32*  m_pUnkEC;          // 0xEC [ m_iNumFrustums ]
	TUINT32*  m_pUnkF0;          // 0xF0 [ m_iNumFrustums ]
	TUINT32*  m_pUnkF4;          // 0xF4 [ m_iNumFrustums ]
	TUINT32   m_uiUnkF8;         // 0xF8
	TUINT32   m_uiUnkFC;         // 0xFC
	TINT      m_iUnk100;         // 0x100
	TFLOAT    m_fUnk104;         // 0x104
	TINT      m_iUnk108;         // 0x108
	TUINT16   m_uiUnk10C;        // 0x10C
	TUINT8    m_aUnk10E[ 0x26 ]; // 0x10E
	TUINT32   m_uiUnk134;        // 0x134
	TTexture* m_pTexture;        // 0x138
	TUINT32   m_aUnk13C[ 1 ];    // 0x13C
};

TOSHI_NAMESPACE_END
