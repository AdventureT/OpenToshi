#include "pch.h"

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// This file includes the entrypoint so set all the settings before including it

#include "Toshi.h"

#include <catch2/catch_session.hpp>

int TMain(int argc, char** argv)
{
	int result = Catch::Session().run(argc, argv);

	return result;
}