#pragma once

#ifdef TOSHI_SKU_WINDOWS
#  include TOSHI_MULTIPLATFORM(TDLL)
#else  // TOSHI_SKU_WINDOWS
class TDLL;
#endif // TOSHI_SKU_WINDOWS
