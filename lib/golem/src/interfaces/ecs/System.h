#pragma once

#include <interfaces/ecs/Entity.h>
#include <interfaces/ecs/EntityManager.h>

namespace ecs {

class EntityMemoryPool;

template <typename Type>
class System {
    friend class SystemFactory;

public:
    void update()
    {
        m_system.update(
            m_entity_pool,
            m_manager.entities());
    }

private:
    template <typename... Args>
    System(
        EntityMemoryPool& entity_pool,
        EntityManager& manager,
        Args&&... args)
        : m_entity_pool(entity_pool)
        , m_manager(manager)
        , m_system(std::forward<Args>(args)...)
    {
    }

    EntityMemoryPool& m_entity_pool;
    EntityManager& m_manager;
    Type m_system;
};

}
