#pragma once
#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/TToneMap_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/TToneMap_SDL.h"
#endif // TOSHI_RENDERER_OPENGL
