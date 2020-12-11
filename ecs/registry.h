#ifndef REGISTRY_H
#define REGISTRY_H

#include "ecs/common.h"
#include "ecs/component.h"
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
public:
    Registry()  = default;
    ~Registry() = default;

    // Register component type.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    void RegisterComponent();

    // Add a component to an entity.
    // A type should be registered in the Registry, otherwise std::runtime_error is thrown.
    template <typename ComponentT>
    ComponentT& AddComponent(Entity entity);

    // Get a reference to a component for an entity. A type should be registered in the World and
    // an entity should have ComponentT component, otherwise std::runtime_error is thrown.
    template <typename ComponentT>
    ComponentT& GetComponent(Entity entity) { return GetComponentStorage<ComponentT>().GetComponent(entity); }

    // Get total number of components of a given ComponentT.
    // A type should be registered in the Registry and otherwise std::runtime_error is being thrown.
    template <typename ComponentT>
    size_t GetNumComponents() { return GetComponentStorage<ComponentT>().Size(); }

     // Register a system.
     // It is not possible to have two systems of the same type in Registry as they are indexed by their type.
    template <typename SystemT, typename... Args>
    void RegisterSystem(Args&&... args);

    // Run one step of an execution.
    // During one step of an execution each registered system is called exactly
    // once respecting, the execution order constratints specified by the user.
    void Run();

    // Wipe out all the component and systems. 
    // Registry to its initial state as if nothing has been registered and executed.
    void Reset();

    // Get a reference to a system.
    template <typename SystemT>
    SystemT& GetSystem();

private:
    // Get reference to a component storage of a specified type.
    // If type is not registered, throws std::runtime_error.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    StorageT& GetComponentStorage();

    // Each time entity space is out, we extend an array by this number of elements.
    static constexpr uint32_t ENTITY_SIZE_INCREMENT = 128;

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

} // namespace ecs

#endif
