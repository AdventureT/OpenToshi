#pragma once
#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/T2Material_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/T2Material_SDL.h"
#endif // TOSHI_RENDERER_OPENGL
