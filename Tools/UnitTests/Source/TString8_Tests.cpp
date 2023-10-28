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

TEST_CASE("String concatination", "[TString8]")
{
	TString8 str("Test");
	str.Concat(" 123");
	REQUIRE(str == "Test 123");
}

TEST_CASE("String formatting", "[TString8]")
{
	TString8 test = TString8::Format("%s %d %i", "Hello", 123, 321);
	REQUIRE(test == "Hello 123 321");
}