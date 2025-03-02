#pragma once

#include <interfaces/ecs/Entity.h>

#include <interfaces/ecs/components/AnchorComponent.h>
#include <interfaces/ecs/components/BorderComponent.h>
#include <interfaces/ecs/components/DrawComponent.h>
#include <interfaces/ecs/components/FillComponent.h>
#include <interfaces/ecs/components/KeyboardInputComponent.h>
#include <interfaces/ecs/components/MouseAreaComponent.h>
#include <interfaces/ecs/components/PositionComponent.h>
#include <interfaces/ecs/components/SizeComponent.h>
#include <interfaces/ecs/components/TextureComponent.h>
#include <interfaces/ecs/components/UpdateComponent.h>

#include <interfaces/ecs/components/MouseAreaComponent.h>

#include <optional>
#include <string>
#include <tuple>
#include <vector>

namespace ecs {

using ComponentVectorTuple = std::tuple<
    std::vector<AnchorTopComponent>,
    std::vector<AnchorRightComponent>,
    std::vector<AnchorLeftComponent>,
    std::vector<AnchorBottomComponent>,
    std::vector<BorderComponent>,
    std::vector<DrawComponent>,
    std::vector<FillComponent>,
    std::vector<OnKeyPressedComponent>,
    std::vector<OnClickedComponent>,
    std::vector<OnPositionChangedComponent>,
    std::vector<OnPressedComponent>,
    std::vector<OnReleasedComponent>,
    std::vector<TextureComponent>,
    std::vector<UpdateComponent>,
    std::vector<PositionComponent>,
    std::vector<SizeComponent>,
    std::vector<ZPositionComponent>>;

class EntityMemoryPool {
public:
    EntityMemoryPool(size_t max_entities);

    size_t maxEntitiesSize() const;

    std::optional<Entity> addEntity(
        std::string tag);

    void removeEntity(Entity entity);

    template <typename ComponentData>
    void addComponent(Entity entity, ComponentData data)
    {
        auto& component = std::get<std::vector<Component<ComponentData>>>(m_pool)[entity];
        component.data = std::move(data);
        component.active = true;
    }

    template <typename ComponentData>
    ComponentData& getComponent(Entity entity)
    {
        return std::get<std::vector<Component<ComponentData>>>(m_pool)[entity].data;
    }

    template <typename ComponentData>
    const ComponentData& getComponent(Entity entity) const
    {
        return std::get<std::vector<Component<ComponentData>>>(m_pool)[entity].data;
    }

    template <typename Component>
    bool hasComponent(Entity entity) const
    {
        return std::get<std::vector<Component>>(m_pool)[entity].active;
    }

private:
    size_t m_max_entities;
    ComponentVectorTuple m_pool;
    std::vector<std::string> m_tags;
    std::vector<bool> m_active;
    size_t m_next_inserted;
};

}
