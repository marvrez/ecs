#include "ecs/ecs.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch2/catch.hpp"

unsigned Factorial(unsigned int number) { return number <= 1 ? number : Factorial(number-1)*number; }
TEST_CASE("Factorials are computed", "[factorial]") {
    REQUIRE(Factorial(1) == 1);
    REQUIRE(Factorial(2) == 2);
    REQUIRE(Factorial(3) == 6);
    REQUIRE(Factorial(10) == 3628800);
}
