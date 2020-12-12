#ifndef ENTITY_H
#define ENTITY_H

#include "ecs/common.h"

#include <algorithm>

namespace ecs
{

class Registry;

// Represents a collection of entities and provides filtering operations.
// EntityManager is primarily designed to be used as a result of entity queries for
// systems. EntityManager provides and API to filter entities based on binary predicate.
class EntityManager {
public:
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;
    EntityManager(EntityManager&& rhs) : m_entities(std::move(rhs.m_entities)) {}

    // Filter a set of entities inplace.
    template <typename F>
    EntityManager& FilterInPlace(F&& f);
    // Filter a set of entities and create a new one as a result.
    // Regular version.
    template <typename F>
    EntityManager Filter(F&& f) const&;
    // R-value version reusing internal storage of a temporary object.
    template <typename F>
    EntityManager Filter(F&& f) &&;

    // Return entities.
    const std::vector<Entity>& Entities() const { return m_entities; }

private:
    EntityManager(const std::vector<Entity>& entities) : m_entities(entities) {}
    EntityManager(std::vector<Entity>&& entities) : m_entities(std::move(entities)) {}

    std::vector<Entity> m_entities;
    friend class EntityQuery;
};

class EntityQuery {
public:
    explicit EntityQuery(Registry& registry) noexcept : m_registry(registry) {}
    // Do not allow copies.
    EntityQuery(const EntityQuery&) = delete;
    EntityQuery operator=(const EntityQuery&) = delete;

    // Get the EntityManager containing all entities in the registry.
    // The EntityManagers further provide filtering functionality on its entities.
    EntityManager operator()() const;
private:
    Registry& m_registry;
};

template <typename F>
inline EntityManager& EntityManager::FilterInPlace(F&& f)
{
    auto new_end = std::partition(m_entities.begin(), m_entities.end(), std::forward<F>(f));
    m_entities.resize(std::distance(m_entities.begin(), new_end));
    return *this;
}

template <typename F>
inline EntityManager EntityManager::Filter(F&& f) &&
{
    auto new_end = std::partition(m_entities.begin(), m_entities.end(), std::forward<F>(f));
    m_entities.resize(std::distance(m_entities.begin(), new_end));
    return EntityManager(std::move(m_entities));
}

template <typename F>
inline EntityManager EntityManager::Filter(F&& f) const&
{
    auto new_end = std::partition(m_entities.begin(), m_entities.end(), std::forward<F>(f));
    return EntityManager(std::vector<Entity>(m_entities.begin(), new_end));
}

} // namespace ecs

#endif
