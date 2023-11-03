#include "pch.h"
#include <catch2/catch_test_macros.hpp>

TOSHI_NAMESPACE_USING


TOBJECT(TTest, TTask, TTRUE)
{

};

TOBJECT(TTest2, TObject, TFALSE)
{

};

TEST_CASE("IsA and IsExactly", "[TObject]")
{
	TTest test;

	SECTION("IsA") {
		REQUIRE(test.IsA(TGetClass(TTask)));
		REQUIRE(test.IsA(TGetClass(TTest)));
		REQUIRE_FALSE(test.IsA(TGetClass(TTest2)));
	}

	SECTION("IsExactly") {
		REQUIRE_FALSE(test.IsExactly(TGetClass(TTask)));
		REQUIRE_FALSE(test.IsExactly(TGetClass(TTest2)));
		REQUIRE(test.IsExactly(TGetClass(TTest)));
	}
}

TEST_CASE("Creating Object", "[TObject]")
{
	SECTION("Instantiable") {
		TTest test;

		REQUIRE(TNULL != TTest::CreateTObject());
		REQUIRE(TNULL != TTest::CreateTObjectInPlace(&test));
	}

	SECTION("Not instantiable") {
		TTest2 test;

		REQUIRE(TNULL == TTest2::CreateTObject());
		REQUIRE(TNULL == TTest2::CreateTObjectInPlace(&test));
	}
}

TEST_CASE("GetClass", "[TObject]")
{
	TTest test;
	
	REQUIRE(test.IsExactly(TGetClass(TTest)));
	REQUIRE(test.IsExactly(test.GetClass()));
}