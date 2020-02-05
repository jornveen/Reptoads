#include "catch/catch.hpp"

TEST_CASE("Does it catch2 compile and run?")
{
	printf("Beginning the Catch2 Test cases!!!");
	for (int i = 0; i < 10; ++i) {
		CHECK(i < 10);
	}
}
