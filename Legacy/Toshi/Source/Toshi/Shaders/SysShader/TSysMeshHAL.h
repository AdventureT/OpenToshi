#pragma once
#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/Shaders/SysShader/TSysMeshHAL_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/Shaders/SysShader/TSysMeshHAL_SDL.h"
#endif // TOSHI_RENDERER_OPENGL
