#include "registry.h"

namespace ecs
{

void Registry::Run()
{
    m_executor.run(m_taskflow);
    m_executor.wait_for_all();
}

void Registry::Reset()
{
    m_entities.clear();
    m_components.clear();
    m_systems.clear();
}

Registry::EntityBuilder Registry::CreateEntity()
{
    // Each time entity space is out, we extend an array by this number of elements.
    static constexpr uint32_t ENTITY_SIZE_INCREMENT = 128;

    std::lock_guard<std::mutex> lock(m_entity_mtx);

    auto id = INVALID_ENTITY;
    // Look for free entity in an existing array.
    for (size_t i = 0; i < m_entities.size(); ++i) {
        if (!m_entities[i]) {
            id = static_cast<Entity>(i);
            break;
        }
    }
    // If entity array is full, resize and add new entities.
    if (id == INVALID_ENTITY) {
        // Add the new entity at the old end position.
        const auto prev_size = m_entities.size();
        id = static_cast<Entity>(prev_size);
        // Set added elements to false. I.e., these spots are free.
        m_entities.resize(prev_size + ENTITY_SIZE_INCREMENT);
        std::fill(m_entities.begin() + prev_size, m_entities.end(), false);
    }
    // Mark entity as existing.
    m_entities[id] = true;
    return Registry::EntityBuilder(id, *this);
}

void Registry::DestroyEntity(Entity entity)
{
    std::lock_guard<std::mutex> clock(m_component_mtx), elock(m_entity_mtx);
    for (auto& c : m_components) {
        if (c.second->HasComponent(entity)) c.second->RemoveComponent(entity);
    }
    m_entities[entity] = false;
}

} // namespace ecs
