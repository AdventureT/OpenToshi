#pragma once
#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/TTexture_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/TTexture_SDL.h"
#endif // TOSHI_RENDERER_OPENGL
