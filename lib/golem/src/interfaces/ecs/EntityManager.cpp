#include <interfaces/ecs/EntityManager.h>

#include <interfaces/ecs/EntityMemoryPool.h>

namespace ecs {

EntityManager::EntityManager(EntityMemoryPool& memory_pool)
    : m_memory_pool(memory_pool)
{
    m_entities.reserve(m_memory_pool.maxEntitiesSize());
}

std::optional<Entity> EntityManager::addEntity(std::string tag)
{
    auto entity = m_memory_pool.addEntity(std::move(tag));
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

void EntityManager::removeEntity(ecs::Entity entity)
{
    auto it = std::find(m_entities.begin(), m_entities.end(), entity);

    if (it == m_entities.end()) {
        return;
    }
    m_memory_pool.removeEntity(entity);
    m_entities.erase(it);
}

}
