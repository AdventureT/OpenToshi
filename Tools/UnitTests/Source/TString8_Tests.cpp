#include "pch.h"
#include <catch2/catch_test_macros.hpp>

TOSHI_NAMESPACE_USING

TEST_CASE("String compare", "[TString8]")
{
	TString8 str("Test");
	REQUIRE(str.Compare("Test", -1) == 0);
	REQUIRE(str.Compare("test", -1) > 0);
	REQUIRE(str.CompareNoCase("test", -1) == 0);
	REQUIRE(str[3] == 't');
	REQUIRE_FALSE(str.IsIndexValid(5));
}