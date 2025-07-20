#pragma once
#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/TFXAA_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/TFXAA_SDL.h"
#endif // TOSHI_RENDERER_OPENGL
