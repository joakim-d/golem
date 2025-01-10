#include <ecs/systems/AnchoringSystem.h>

#include <ecs/components/AnchorComponent.h>
#include <ecs/components/PositionComponent.h>
#include <ecs/components/SizeComponent.h>

#include <ecs/EntityMemoryPool.h>

namespace ecs {

static int getYPosition(
    EntityMemoryPool& pool,
    Entity entity,
    VerticalPosition position)
{
    if (position == Top) {
        return pool.getComponent<Position>(entity).y;
    } else {
        return pool.getComponent<Position>(entity).y + pool.getComponent<Size>(entity).h;
    }
}

static int getXPosition(
    EntityMemoryPool& pool,
    Entity entity,
    HorizontalPosition position)
{
    if (position == Left) {
        return pool.getComponent<Position>(entity).x;
    } else {
        return pool.getComponent<Position>(entity).x + pool.getComponent<Size>(entity).w;
    }
}

void AnchoringSystem::update(
    EntityMemoryPool& pool,
    const std::vector<Entity>& entities)
{
    for (auto entity : entities) {
        auto& position = pool.getComponent<Position>(entity);
        auto& size = pool.getComponent<Size>(entity);

        if (pool.hasComponent<AnchorTopComponent>(entity)) {
            const auto& anchor = pool.getComponent<AnchorTop>(entity);

            position.y = getYPosition(pool, anchor.entity, anchor.anchorTo) + anchor.margin;
        }

        if (pool.hasComponent<AnchorBottomComponent>(entity)) {
            const auto& anchor = pool.getComponent<AnchorBottom>(entity);
            size.h = getYPosition(pool, anchor.entity, anchor.anchorTo) - position.y - anchor.margin;
        }

        if (pool.hasComponent<AnchorLeftComponent>(entity)) {
            const auto& anchor = pool.getComponent<AnchorLeft>(entity);

            position.x = getXPosition(pool, anchor.entity, anchor.anchorTo) + anchor.margin;
        }

        if (pool.hasComponent<AnchorRightComponent>(entity)) {
            const auto& anchor = pool.getComponent<AnchorRight>(entity);

            size.w = getXPosition(pool, anchor.entity, anchor.anchorTo) - position.x - anchor.margin;
        }
    }
}

}
