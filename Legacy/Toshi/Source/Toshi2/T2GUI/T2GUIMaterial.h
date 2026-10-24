#pragma once
#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/T2GUI/T2GUIMaterial_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/T2GUI/T2GUIMaterial_SDL.h"
#endif // TOSHI_RENDERER_OPENGL
