#include "pch.h"
#include <catch2/catch_test_macros.hpp>

TOSHI_NAMESPACE_USING

struct TestStruct
{
	TBOOL operator==(TestStruct other)
	{
		return a == other.a && b == other.b && str == other.str;
	}

	int a;
	float b;
	TString8 str;
};

TEST_CASE("GetFirst and GetSecond", "[T2Pair]")
{
	T2Pair<int, float> pair = T2Pair<int, float>(1, 1.0f);
	REQUIRE(pair.GetFirst() == 1);
	REQUIRE(pair.GetSecond() == 1.0f);
}

TEST_CASE("GetFirst and GetSecond with Struct", "[T2Pair]")
{
	TestStruct test{ 1, 1.0f, "Hey" };
	T2Pair<int, TestStruct> pair = T2Pair<int, TestStruct>(1, { 1, 1.0f, TString8("Hey") });
	REQUIRE(pair.GetFirst() == 1);
	REQUIRE(pair.GetSecond() == test);

	SECTION("Test with different pair") {
		pair = T2Pair<int, TestStruct>(2, { 4, 2.0f, TString8("Bye") });

		REQUIRE(pair.GetFirst() == 2);
		REQUIRE_FALSE(pair.GetSecond() == test);
	}
}