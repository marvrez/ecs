#include "ecs/ecs.h"

#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch2/catch.hpp"

#include <stdexcept>

struct TestData  { float x; };
struct TestData1 { float x, y; };
struct TestData2 { float x, y, z; };

struct TestSystem : public ecs::System {
    virtual void Run(ecs::ComponentAccess& access, ecs::EntityQuery& entity_query, tf::Subflow& subflow) override {}
};

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

TEST_CASE("Register system", "[registry|system]")
{
    using namespace ecs;
    Registry registry;
    REQUIRE_NOTHROW(registry.RegisterSystem<TestSystem>());
}

TEST_CASE("Register same system", "[registry|system]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_NOTHROW(registry.RegisterSystem<TestSystem>());
    REQUIRE_THROWS_AS(registry.RegisterSystem<TestSystem>(), std::runtime_error);
}

TEST_CASE("Get system", "[registry|system]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_THROWS_AS(registry.GetSystem<TestSystem>(), std::runtime_error);
    REQUIRE_NOTHROW(registry.RegisterSystem<TestSystem>());
    REQUIRE_NOTHROW(registry.GetSystem<TestSystem>());
}

TEST_CASE("Create entity", "[registry|entity]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_NOTHROW(registry.RegisterComponent<TestData>());
    REQUIRE_NOTHROW(registry.RegisterComponent<TestData1>());
    REQUIRE_NOTHROW(registry.CreateEntity().AddComponent<TestData>().AddComponent<TestData1>().Build());
}

TEST_CASE("Create same entities", "[registry|entity]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_NOTHROW(registry.RegisterComponent<TestData>());
    REQUIRE_THROWS_AS(registry.CreateEntity().AddComponent<TestData>().AddComponent<TestData>().Build(), std::runtime_error);
}

TEST_CASE("Destroy entity", "[registry|entity]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_NOTHROW(registry.RegisterComponent<TestData>());
    auto entity = registry.CreateEntity().AddComponent<TestData>().Build();
    REQUIRE_NOTHROW(registry.HasComponent<TestData>(entity));

    REQUIRE_NOTHROW(registry.GetComponent<TestData>(entity));
    REQUIRE_NOTHROW(registry.DestroyEntity(entity));
    REQUIRE_THROWS_AS(registry.GetComponent<TestData>(entity), std::runtime_error);
}

TEST_CASE("Counting system", "[registry|system]")
{
    using namespace ecs;
    Registry registry;

    REQUIRE_NOTHROW(registry.RegisterComponent<TestData>());
    REQUIRE_NOTHROW(registry.RegisterComponent<TestData1>());

    class CounterSystem : public System {
    private:
        int& m_count0, &m_count1, &m_count2;
    public:
        CounterSystem(int& count0, int& count1, int& count2) : m_count0(count0), m_count1(count1), m_count2(count2) { }

        void Run(ComponentAccess& access, EntityQuery& entity_query, tf::Subflow& subflow) override
        {
            const auto& td  = access.Read<TestData>();
            const auto& td1 = access.Read<TestData1>();
            m_count0 = entity_query().Filter([&td] (Entity e) { return td.HasComponent(e);  }).Entities().size();
            m_count1 = entity_query().Filter([&td1](Entity e) { return td1.HasComponent(e); }).Entities().size();
            m_count2 = entity_query().Filter([&td, &td1](Entity e) { return td.HasComponent(e) && td1.HasComponent(e); }).Entities().size();
        }
    };

    constexpr int EXPECTED_NUM_ENTITIES = 1024;
    for (int i = 0; i < EXPECTED_NUM_ENTITIES; ++i) {
        if (i & 1) REQUIRE_NOTHROW(registry.CreateEntity().AddComponent<TestData>().Build());
        else REQUIRE_NOTHROW(registry.CreateEntity().AddComponent<TestData1>().Build());
    }

    int count0 = -1, count1 = -1, count2 = -1;
    REQUIRE_NOTHROW(registry.RegisterSystem<CounterSystem>(count0, count1, count2));

    REQUIRE_NOTHROW(registry.Run());
    REQUIRE(count0 == EXPECTED_NUM_ENTITIES / 2);
    REQUIRE(count1 == EXPECTED_NUM_ENTITIES / 2);
    REQUIRE(count2 == 0);
}
