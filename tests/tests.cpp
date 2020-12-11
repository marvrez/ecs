#include "ecs/ecs.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch2/catch.hpp"

#include <stdexcept>

struct TestData  { float x; };
struct TestData1 { float x, y; };
struct TestData2 { float x, y, z; };

TEST_CASE("Create registry", "[registry|component]")
{
    using namespace ecs;
    Registry registry;
}

TEST_CASE("Register component", "[registry|component]")
{
    using namespace ecs;
    Registry registry;
    REQUIRE_NOTHROW(registry.RegisterComponent<TestData>());
}

TEST_CASE("Register same component type", "[registry|component]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_NOTHROW(registry.RegisterComponent<TestData>());
    REQUIRE_THROWS_AS(registry.RegisterComponent<TestData>(), std::runtime_error);
}

TEST_CASE("Register multiple component types", "[registry|component]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_NOTHROW(registry.RegisterComponent<TestData>());
    REQUIRE_NOTHROW(registry.RegisterComponent<TestData1>());
    REQUIRE_NOTHROW(registry.RegisterComponent<TestData2>());
}
