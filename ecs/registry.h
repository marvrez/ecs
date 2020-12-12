#ifndef REGISTRY_H
#define REGISTRY_H

#include "ecs/common.h"
#include "ecs/component.h"
#include "ecs/entity.h"
#include "ecs/system.h"

#include <thirdparty/taskflow/taskflow/taskflow.hpp>

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <typeindex>

namespace ecs
{

// Provides primary ECS interface for the user.
// Registry hosts all ECS data and provides an interface to all clients.
class Registry {
private:
    // Helper class for easier entities construction.
    // Registry returns EntityBuilder as a result of CreateEntity method, which conveniently allows for chain-calls.
    // E.g., registry.CreateEntity().AddComponent<Foo>().AddComponent<Bar>().Build();
    class EntityBuilder {
    public:
        EntityBuilder(EntityBuilder&) = delete;
        EntityBuilder& operator=(EntityBuilder&) = delete;

        // Add component of a given type.
        template <typename ComponentT>
        EntityBuilder& AddComponent()
        {
            m_registry.AddComponent<ComponentT>(m_entity);
            return *this;
        }

        // Build entity (i.e., return its id).
        Entity Build() const noexcept { return m_entity; }
    private:
        // Only Registry can create EntityBuiler's.
        EntityBuilder(Entity entity, Registry& registry) noexcept : m_entity(entity), m_registry(registry) {}

        // Entity of interest.
        Entity m_entity = INVALID_ENTITY;
        Registry& m_registry;

        friend class Registry;
    };
public:
    Registry()  = default;
    ~Registry() = default;

    // Create an empty entity and returns a builder instance which can be used to add components.
    // E.g., registry.CreateEntity().AddComponent<Foo>().AddComponent<Bar>().Build().
    EntityBuilder CreateEntity();

    // Destroys an entity along with its components.
    void DestroyEntity(Entity entity);

    // Register component type.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    void RegisterComponent();

    // Add a component to an entity.
    // A type should be registered in the Registry, otherwise std::runtime_error is thrown.
    template <typename ComponentT>
    ComponentT& AddComponent(Entity entity);

    // Get a reference to a component for an entity. A type should be registered in the Registry and
    // an entity should have ComponentT component, otherwise std::runtime_error is thrown.
    template <typename ComponentT>
    ComponentT& GetComponent(Entity entity) { return GetComponentStorage<ComponentT>().GetComponent(entity); }

    // Check if an entity has a component of a given type.
    // A type must be registered in the Registry, otherwise std::runtime_error is thrown.
    template <typename ComponentT>
    bool HasComponent(Entity entity) {  return GetComponentStorage<ComponentT>().HasComponent(entity); }

    // Get total number of components of a given ComponentT.
    // A type should be registered in the Registry and otherwise std::runtime_error is being thrown.
    template <typename ComponentT>
    size_t GetNumComponents() { return GetComponentStorage<ComponentT>().Size(); }

    // Run one step of an execution.
    // During one step of an execution each registered system is called exactly
    // once respecting, the execution order constratints specified by the user.
    void Run();

    // Wipe out all the component and systems. 
    // Registry to its initial state as if nothing has been registered and executed.
    void Reset();

    // Register a system.
    // It is not possible to have two systems of the same type in Registry as they are indexed by their type.
    template <typename SystemT, typename... Args>
    void RegisterSystem(Args&&... args);

    // Get a reference to a system.
    template <typename SystemT>
    SystemT& GetSystem();

    // Make one system SystemT0 run before another system SystemT1.
    // By default, systems can execute in arbitrary orders -- or even in parallel!
    // Precede() sets an order of the system execution.
    template <typename SystemT0, typename SystemT1>
    void Precede();

private:
    // Get reference to a component storage of a specified type.
    // If type is not registered, throws std::runtime_error.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    StorageT& GetComponentStorage();

    // Data associated with a system.
    struct SystemInvocation {
        tf::Task                task;
        std::unique_ptr<System> system;
    };

    // Entity array: true if entity exists
    std::mutex        m_entity_mtx;
    std::vector<bool> m_entities;
    // Component arrays
    std::mutex    m_component_mtx;
    std::unordered_map<std::type_index, std::unique_ptr<ComponentStorageInterface>> m_components;
    // Systems
    std::mutex m_system_mtx;
    std::unordered_map<std::type_index, SystemInvocation> m_systems;

    tf::Taskflow m_taskflow;
    tf::Executor m_executor;

    friend class EntityQuery;
    friend class ComponentAccess;
};

// An interface providing access to components for System subclasses, guarding the Registry from unattended access.
class ComponentAccess {
public:
    // Request component storage for write access.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    StorageT& Write() { return m_registry.GetComponentStorage<ComponentT>(); }
    // Request component storage for read access.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    const StorageT& Read() const { return m_registry.GetComponentStorage<ComponentT>(); }
private:
    // Only registry can create these objects.
    explicit ComponentAccess(Registry& registry) noexcept : m_registry(registry) {}
    Registry& m_registry;
    friend class Registry;
};

template <typename ComponentT, typename StorageT>
inline StorageT& Registry::GetComponentStorage()
{
    const auto tidx = TypeIndex<ComponentT>();
    assert(m_components.find(tidx) != m_components.cend());
    auto storage = static_cast<StorageT*>(m_components[tidx].get());
    return *storage;
}

template <typename ComponentT, typename StorageT>
inline void Registry::RegisterComponent()
{
    std::lock_guard<std::mutex> lock(m_component_mtx);
    const auto tidx = TypeIndex<ComponentT>();
    if (m_components.find(tidx) != m_components.cend()) throw std::runtime_error("Registry: the specified component type is already registered.");
    m_components.emplace(tidx, std::move(std::make_unique<StorageT>()));
}

template <typename ComponentT>
inline ComponentT& Registry::AddComponent(Entity entity)
{
    std::lock_guard<std::mutex> lock(m_component_mtx);
    return GetComponentStorage<ComponentT>().AddComponent(entity);
}

template <typename SystemT, typename... Args>
inline void Registry::RegisterSystem(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_system_mtx);
    const auto tidx = TypeIndex<SystemT>();
    if (m_systems.find(tidx) != m_systems.cend()) throw std::runtime_error("Registry: system type already registered");

    SystemInvocation invoke;
    invoke.system = std::make_unique<SystemT>(std::forward<Args>(args)...);
    invoke.task   = m_taskflow.emplace([system=invoke.system.get(), this](tf::Subflow& subflow) {
        ComponentAccess access(*this);
        EntityQuery     query(*this);
        system->Run(access, query, subflow);
    });

    m_systems.emplace(tidx, std::move(invoke));
}

template <typename SystemT>
inline SystemT& Registry::GetSystem()
{
    const auto tidx = TypeIndex<SystemT>();
    auto system = m_systems.find(tidx);
    if (system == m_systems.cend()) throw std::runtime_error("Registry: the specified system type was not found");
    return reinterpret_cast<SystemT&>(*system->second.system.get());
}

template <typename SystemT0, typename SystemT1>
inline void Registry::Precede()
{
    const auto tidx0 = TypeIndex<SystemT0>(), tidx1 = TypeIndex<SystemT1>();
    auto s0 = m_systems.find(tidx0), s1 = m_systems.find(tidx1);
    if (s0 == m_systems.cend() || s1 == m_systems.cend()) throw std::runtime_error("Registry: one of the specified system types were not found");
    s0->second.task.precede(s1->second.task);
}

} // namespace ecs

#endif
