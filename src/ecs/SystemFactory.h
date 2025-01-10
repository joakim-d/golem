#pragma once

#include <utility>

namespace ecs {

class EntityMemoryPool;
class EntityManager;

template <class T>
class System;

class SystemFactory {
public:
    SystemFactory(
        EntityMemoryPool& entity_pool,
        EntityManager& manager);

    template <typename T, typename... Args>
    System<T> createSystem(Args&&... args)
    {
        return System<T>(
            m_entity_pool,
            m_manager,
            std::forward<Args>(args)...);
    }

private:
    EntityMemoryPool& m_entity_pool;
    EntityManager& m_manager;
};

}
