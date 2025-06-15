#pragma once

#include <map>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>
#include <utility>

/* Toshi Header Files */
#include "Toshi/Core/Core.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Core/STL.h"
#include "Toshi/Core/Endianness.h"
#include "Toshi/Core/TObject.h"
#include "Toshi/Math/Math.h"
#include "Toshi/Utils/TLog.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Utils/TUtil.h"
#include "Toshi/Thread/TMutexLock.h"
#include "Toshi/Strings/TStringManager.h"

/* Toshi 2.0 Header Files */
#include "Toshi2/Thread/T2MutexLock.h"
#include "Toshi2/T2Flags.h"

/* Platform specific headers */
#ifdef TOSHI_SKU_WINDOWS
#  include <d3d11.h>
#  include <dinput.h>
#  include <Windows.h>
#endif
