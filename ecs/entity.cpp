#include "entity.h"

#include "ecs/registry.h"

namespace ecs
{

EntityManager EntityQuery::operator()() const
{
    std::vector<Entity> entities;
    for (uint32_t i = 0; i < m_registry.m_entities.size(); ++i)
        if (m_registry.m_entities[i]) entities.push_back(i);
    return EntityManager(std::move(entities));
}

} // namespace ecs
