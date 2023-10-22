#include "pch.h"

// TOSHI_TMEMORY_FLAGS allows you to change the mode TMemory initializes with
// Flags_NativeMethods stands for native memory management methods instead of using dlmalloc
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_NativeMethods

// This file includes the entrypoint so set all the settings before including it
#include "Toshi.h"

#define CATCH_AMALGAMATED_CUSTOM_MAIN
#include "Catch2/catch_amalgamated.hpp"

int TMain(int argc, char** argv)
{
	int result = Catch::Session().run(argc, argv);

	return result;
}

TOSHI_NAMESPACE_USING

TEST_CASE("GetFirst()", "[T2Pair]")
{
	auto pair = T2Pair<int, float>(1, 1.0f);
	REQUIRE(pair.GetFirst() == 1);
}

TEST_CASE("GetSecond()", "[T2Pair]")
{
	auto pair = T2Pair<int, float>(1, 1.0f);
	REQUIRE(pair.GetSecond() == 1.0f);
}

TEST_CASE("String compare", "[TString8]")
{
	TString8 str("Test");
	REQUIRE(str.Compare("Test", -1) == 0);
	REQUIRE(str.Compare("test", -1) > 0);
	REQUIRE(str.CompareNoCase("test", -1) == 0);
	REQUIRE(str[3] == 't');
	REQUIRE_FALSE(str.IsIndexValid(5));
}