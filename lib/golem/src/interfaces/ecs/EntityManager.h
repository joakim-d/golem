#pragma once

#include <interfaces/ecs/Entity.h>
#include <interfaces/ecs/EntityMemoryPool.h>

#include <optional>
#include <string>
#include <vector>

namespace ecs {

class EntityManager {
public:
    EntityManager(EntityMemoryPool& memory_pool);

    std::optional<Entity> addEntity(std::string tag);

    void removeEntity(Entity entity);

    const std::vector<Entity>& entities() const;

    template <typename ComponentData>
    void addComponent(Entity entity, ComponentData data)
    {
        m_memory_pool.addComponent(entity, data);
    }

    template <typename ComponentData>
    bool hasComponent(Entity entity)
    {
        return m_memory_pool.hasComponent<Component<ComponentData>>(entity);
    }

    template <typename ComponentData>
    const ComponentData& getComponent(Entity entity) const
    {
        return m_memory_pool.getComponent<ComponentData>(entity);
    }

private:
    EntityMemoryPool& m_memory_pool;
    std::vector<Entity> m_entities;
};

}
