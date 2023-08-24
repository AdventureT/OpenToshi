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
#include "Toshi/Typedefs.h"
#include "Toshi/Memory/TMemory.h"
#include "Toshi/Core/STL.h"
#include "Toshi/Core/Core.h"
#include "Toshi/Core/TObject.h"
#include "Toshi/Core/TTask.h"
#include "Toshi/Math/Math.h"
#include "Toshi/Utils/TLog.h"
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Utils/TUtil.h"
#include "Toshi/Thread/TMutexLock.h"
#include "Toshi/Strings/TStringManager.h"
#include "Toshi/Core/TDebug.h"
#include <Toshi/Core/TSystem.h>

/* Toshi 2.0 Header Files */
#include "Toshi2/Thread/T2MutexLock.h"
#include "Toshi2/T2Flags.h"

/* Platform specific headers */
#ifdef TOSHI_SKU_WINDOWS
	#include <Windows.h>
#endif