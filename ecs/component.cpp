#include "component.h"

#include "ecs/registry.h"

namespace ecs
{

template <typename ComponentT, typename StorageT>
inline StorageT& ComponentAccess::Write()
{
    return m_registry.GetComponentStorage<ComponentT>();
}

template <typename ComponentT, typename StorageT>
inline const StorageT& ComponentAccess::Read() const
{
    m_registry.GetComponentStorage<ComponentT>();
}

} // namespace ecs
