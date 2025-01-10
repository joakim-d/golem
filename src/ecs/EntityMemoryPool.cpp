#include <ecs/EntityMemoryPool.h>

namespace ecs {

EntityMemoryPool::EntityMemoryPool(size_t max_entities)
    : m_max_entities(max_entities)
    , m_next_inserted(0)
{
    std::apply(
        [max_entities](auto&... components) {
            (..., components.resize(max_entities));
        },
        m_pool);

    m_tags.resize(max_entities);
    m_active.resize(max_entities);
}

size_t EntityMemoryPool::maxEntitiesSize() const
{
    return m_max_entities;
}

std::optional<Entity> EntityMemoryPool::addEntity(
    const std::string& tag)
{
    Entity entity;
    for (entity = m_next_inserted; entity < m_max_entities; ++entity) {
        if (!m_active[entity]) {
            // reset components
            std::apply(
                [entity](auto&... components) {
                    (..., components[entity].reset());
                },
                m_pool);
            m_active[entity] = true;
            m_next_inserted = entity + 1;
            return entity;
        }
    }

    return std::nullopt;
}

}
