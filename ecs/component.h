#ifndef COMPONENT_H
#define COMPONENT_H

#include "ecs/common.h"

#include <algorithm>
#include <unordered_map>
#include <vector>

namespace ecs
{

class Registry;

class ComponentStorageInterface {
public:
    virtual ~ComponentStorageInterface() = 0;
    // Collection size
    virtual size_t Size() const = 0;
    // True if entity has a component in this collection
    virtual bool HasComponent(Entity entity) const = 0;
    // Remove component from entity.
    virtual void RemoveComponent(Entity entity) = 0;
};

// Component storage that stores entities in a packed array.
// Components are stored in a packed array and a hash map is used for entity --> component mapping.
template <typename T>
class PackedComponentStorage : public ComponentStorageInterface {
public:
    PackedComponentStorage() = default;
    ~PackedComponentStorage() override = default;

    PackedComponentStorage(const PackedComponentStorage&) = delete;
    PackedComponentStorage& operator=(const PackedComponentStorage&) = delete;

    PackedComponentStorage(PackedComponentStorage&&);
    PackedComponentStorage&& operator=(PackedComponentStorage&&);

    // Get collection size.
    size_t Size() const override { return m_components.size(); }

    // True if entity has a component in this collection.
    bool HasComponent(Entity entity) const override;

    // Remove component from entity.
    void RemoveComponent(Entity entity) override;

    // Get component for entity, throws std::runtime_error if HasComponent(entity) == false.
    T&       GetComponent(Entity entity);
    const T& GetComponent(Entity entity) const;

    // Add a component to an entity.
    T& AddComponent(Entity entity);

    // Access component by idx.
    T&       operator[](ComponentIndex idx)	        { return m_components[idx]; }
    const T& operator[](ComponentIndex idx) const   { return m_components[idx]; }

private:
    std::unordered_map<Entity, ComponentIndex> m_component_index;
    std::vector<T>                             m_components;
};

// An interface providing access to components for System subclasses, guarding the Registry from unattended access.
class ComponentAccess {
public:
    // Request component storage for write access.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    StorageT& Write();
    // Request component storage for read access.
    template <typename ComponentT, typename StorageT = PackedComponentStorage<ComponentT>>
    const StorageT& Read() const;
private:
    // Only registry can create these objects.
    explicit ComponentAccess(Registry& registry) noexcept : m_registry(registry) {}
    Registry& m_registry;
    friend class Registry;
};

inline ComponentStorageInterface::~ComponentStorageInterface() {}

template <typename T>
inline PackedComponentStorage<T>::PackedComponentStorage(PackedComponentStorage&& rhs)
    : m_component_index(std::move(rhs.m_component_index)), m_components(std::move(rhs.m_components))
{
}

template <typename T>
inline PackedComponentStorage<T>&& PackedComponentStorage<T>::operator=(PackedComponentStorage&& rhs)
{
    m_component_index = std::move(rhs.m_component_index);
    m_components      = std::move(rhs.m_components);
}

template <typename T>
inline bool PackedComponentStorage<T>::HasComponent(Entity entity) const
{
    return m_component_index.find(entity) != m_component_index.cend();
}

template <typename T>
inline T& PackedComponentStorage<T>::AddComponent(Entity entity)
{
    if (HasComponent(entity)) throw std::runtime_error("ComponentCollection: Entity already contains the specific component");
    m_component_index[entity] = m_components.size();
    m_components.emplace_back();
    return m_components.back();
}

template <typename T>
inline const T& PackedComponentStorage<T>::GetComponent(Entity entity) const
{
    if (!HasComponent(entity)) throw std::runtime_error("ComponentCollection: Entity does not contain the specific component");
    const ComponentIndex idx = m_component_index.find(entity)->second;
    return m_components[idx];
}

template <typename T>
inline T& PackedComponentStorage<T>::GetComponent(Entity entity)
{
    if (!HasComponent(entity)) throw std::runtime_error("ComponentCollection: Entity does not contain the specific component");
    const ComponentIndex idx = m_component_index[entity];
    return m_components[idx];
}

template <typename T>
inline void PackedComponentStorage<T>::RemoveComponent(Entity entity)
{
    if (!HasComponent(entity)) throw std::runtime_error("ComponentCollection: Entity does not contain the specified component");
	// Swap index and component of the last item 
    // with the index and component of the entity to be deleted
    if (m_components.size() > 1) {
        const ComponentIndex last_idx = m_components.size() - 1;
        const ComponentIndex free_idx = m_component_index[entity];
        std::swap(m_components[free_idx], m_components[last_idx]);

        for (auto& entity_idx : m_component_index) {
            if (entity_idx.second == last_idx) {
                entity_idx.second = free_idx;
                break;
            }
        }
    }
    // Perform deletion of the entity
    m_component_index.erase(entity);
    m_components.pop_back();
}

} // namespace ecs

#endif
