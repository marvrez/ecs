#ifndef SYSTEM_H
#define SYSTEM_H

#include <thirdparty/taskflow/taskflow/taskflow.hpp>

namespace ecs
{

class EntityQuery;
class ComponentAccess;

// Interface for system implementers.
// Registry talks to registered systems via the System interface by calling
// System::Run() on every registered system every time Registry::Run() is called.
class System {
public:
    virtual ~System() = default;

    // Run single step of System execution.
    // Registry calls this method once per Registry::Run invocation.
    virtual void Run(ComponentAccess& access, EntityQuery& entity_query, tf::Subflow& subflow) = 0;
};

} // namespace ecs

#endif
