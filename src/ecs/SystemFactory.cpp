#include <ecs/SystemFactory.h>

namespace ecs {

SystemFactory::SystemFactory(
    EntityMemoryPool& entity_pool,
    EntityManager& manager)
    : m_entity_pool(entity_pool)
    , m_manager(manager)
{
}

}
