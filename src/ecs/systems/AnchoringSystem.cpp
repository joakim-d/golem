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

        bool hasTop = pool.hasComponent<AnchorTopComponent>(entity);
        bool hasBottom = pool.hasComponent<AnchorBottomComponent>(entity);
        bool hasLeft = pool.hasComponent<AnchorLeftComponent>(entity);
        bool hasRight = pool.hasComponent<AnchorRightComponent>(entity);

        // If both anchors it should be stretched
        if (hasTop && hasBottom) {
            const auto& anchorTop = pool.getComponent<AnchorTop>(entity);
            const auto& anchorBottom = pool.getComponent<AnchorBottom>(entity);
            position.y = getYPosition(pool, anchorTop.entity, anchorTop.anchorTo) + anchorTop.margin;
            size.h = getYPosition(pool, anchorBottom.entity, anchorBottom.anchorTo) - position.y - anchorBottom.margin;
        } else if (hasTop) {
            const auto& anchorTop = pool.getComponent<AnchorTop>(entity);
            position.y = getYPosition(pool, anchorTop.entity, anchorTop.anchorTo) + anchorTop.margin;
        } else if (hasBottom) {
            const auto& anchorBottom = pool.getComponent<AnchorBottom>(entity);
            position.y = getYPosition(pool, anchorBottom.entity, anchorBottom.anchorTo) - anchorBottom.margin - size.h;
        }

        // If both anchors it should be stretched
        if (hasLeft && hasRight) {
            const auto& anchorLeft = pool.getComponent<AnchorLeft>(entity);
            const auto& anchorRight = pool.getComponent<AnchorRight>(entity);
            position.x = getXPosition(pool, anchorLeft.entity, anchorLeft.anchorTo) + anchorLeft.margin;
            size.w = getXPosition(pool, anchorRight.entity, anchorRight.anchorTo) - position.x - anchorRight.margin;
        } else if (hasLeft) {
            const auto& anchorLeft = pool.getComponent<AnchorLeft>(entity);
            position.x = getXPosition(pool, anchorLeft.entity, anchorLeft.anchorTo) + anchorLeft.margin;
        } else if (hasRight) {
            const auto& anchorRight = pool.getComponent<AnchorRight>(entity);
            position.x = getXPosition(pool, anchorRight.entity, anchorRight.anchorTo) - anchorRight.margin - size.h;
        }
    }
}

}
