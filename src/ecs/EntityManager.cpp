#include <ecs/EntityManager.h>

#include <ecs/EntityMemoryPool.h>

namespace ecs {

EntityManager::EntityManager(EntityMemoryPool& memory_pool)
    : m_memory_pool(memory_pool)
{
    m_entities.reserve(m_memory_pool.maxEntitiesSize());
}

std::optional<Entity> EntityManager::addEntity(const std::string& tag)
{
    auto entity = m_memory_pool.addEntity(tag);
    if (!entity.has_value()) {
        return entity;
    }

    m_entities.push_back(*entity);
    return entity;
}

const std::vector<Entity>& EntityManager::entities() const
{
    return m_entities;
}

}
