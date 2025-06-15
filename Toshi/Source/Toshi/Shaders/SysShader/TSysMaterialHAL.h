#pragma once
#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/Shaders/SysShader/TSysMaterialHAL_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/Shaders/SysShader/TSysMaterialHAL_SDL.h"
#endif // TOSHI_RENDERER_OPENGL
