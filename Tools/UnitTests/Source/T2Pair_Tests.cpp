#include "pch.h"
#include <catch2/catch_test_macros.hpp>

TOSHI_NAMESPACE_USING

TEST_CASE("GetFirst and GetSecond", "[T2Pair]")
{
	T2Pair<int, float> pair = T2Pair<int, float>(1, 1.0f);
	REQUIRE(pair.GetFirst() == 1);
	REQUIRE(pair.GetSecond() == 1.0f);
}