#include <ecs/systems/UpdaterSystem.h>

#include <ecs/EntityMemoryPool.h>
#include <ecs/components/UpdateComponent.h>

namespace ecs {

void Updater::update(
    EntityMemoryPool& pool,
    const std::vector<Entity>& entities)
{
    for (auto entity : entities) {
        if (!pool.hasComponent<UpdateComponent>(entity)) {
            continue;
        }
        pool.getComponent<Update>(entity).update();
    }
}

}
