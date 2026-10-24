#pragma once

//-----------------------------------------------------------------------------
// de Blob's TTexture is platform code: a plain class managed by TTextureManager, not a TResource. Barnyard's
// TTexture/TTextureFactory don't exist in de Blob.
//-----------------------------------------------------------------------------
#ifdef TRENDERINTERFACE_DX11
#  include "Platform/DX11/TTexture_DX11.h"
#endif // TRENDERINTERFACE_DX11
