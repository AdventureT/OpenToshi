#pragma once
#include "Math/TVector4.h"


TOSHI_NAMESPACE_START

class TTexture;
class TMaterial;

// A particle material from the particle TRBs
class TParticleMaterial
{
public:
	static TCONSTEXPR_VAR TUINT s_LatestVer = 0x10002;

	// m_iBlendMode
	enum BLENDMODE
	{
		BLENDMODE_NORMAL,   // TSysMaterial::BLENDMODE_1
		BLENDMODE_ADDITIVE, // TSysMaterial::BLENDMODE_ADDITIVE
		BLENDMODE_2,        // TSysMaterial::BLENDMODE_4
	};

	// m_uiFlags
	enum FLAGS : TUINT16
	{
		FLAGS_ALPHATEXTURE = 1 << 0,
		FLAGS_NODEPTHTEST  = 1 << 1,
		FLAGS_4            = 1 << 2, // TMaterial::FLAGS_ALPHAUPDATE
		FLAGS_8            = 1 << 3, // TMaterial flag 0x40000000
		FLAGS_10           = 1 << 4, // TMaterial flag 0x20000000
	};

public:
	void Create();
	void Destroy();

	// $deBlob: FUNCTION 00718540
	TVector4* GetUVRect( TINT a_iID ) const
	{
		TASSERT( a_iID >= 0 && a_iID < m_iNumUVRects );
		return &m_pUVRects[ a_iID ];
	}

public:
	TUINT              m_uiVer;      // 0x00 (the material ID once added to the manager)
	TParticleMaterial* m_pNext;      // 0x04
	TParticleMaterial* m_pPrev;      // 0x08
	const TCHAR*       m_szName;     // 0x0C
	const TCHAR*       m_szTexture;  // 0x10
	TTexture*          m_pTexture;   // 0x14
	TMaterial*         m_pMaterial;  // 0x18
	TINT16             m_iNumUVRects; // 0x1C
	TUINT16            m_uiFlags;    // 0x1E
	TINT               m_iPriority;  // 0x20
	TVector4*          m_pUVRects;   // 0x24 (u1, v1, u2, v2)
	TINT               m_iBlendMode; // 0x28
};

TOSHI_NAMESPACE_END
