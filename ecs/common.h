#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <numeric>
#include <typeindex>
#include <typeinfo>
#include <vector>

namespace ecs
{

using Entity         = std::uint32_t;
using ComponentIndex = std::size_t;
constexpr Entity         INVALID_ENTITY          = ~0u;
constexpr ComponentIndex INVALID_COMPONENT_INDEX = ~0u;

// Compute hasheable index given a type. Returns an std::type_index which can be used in hash maps.
template <typename T>
static inline auto TypeIndex() { return std::type_index(typeid(T)); }

} // namespace ecs

#endif
