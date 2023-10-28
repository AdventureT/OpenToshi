#include "pch.h"
#include <catch2/catch_test_macros.hpp>

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