#include "ToshiPCH.h"
#include "TTexture.h"
#include "TRender.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS_NORUNTIME( TTexture );

// $Barnyard: FUNCTION 006d9340
TTexture::TTexture()
{
	m_eAddressing   = ADDRESSINGMODE_WRAP;
	m_eTextureFlags = 0;
	m_pNameEntry    = TNULL;
}

// $Barnyard: FUNCTION 006d9390
// $Barnyard: FUNCTION 006d93c0
TTexture::~TTexture()
{
	// NOTE: Barnyard deregisters the texture from the texture factory system resource (SYSRESOURCE_TEXTUREFACTORY),
	// de Blob's TRender has no such resource (its textures are TTexture_DX11/TTextureManager)
	TTODO( "de Blob's TTexture" );
}

TBOOL TTexture::Lock( LOCKSTATE& a_rLockState )
{
	return TFALSE;
}

void TTexture::Unlock()
{
}

TBOOL TTexture::CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiMipMapFlags )
{
	return TResource::Create();
}

TOSHI_NAMESPACE_END
