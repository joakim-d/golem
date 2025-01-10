#pragma once

#include <SDL2/SDL.h>

#include <graphics/TextureManager.h>
#include <graphics/core/Color.h>

#include <ecs/Entity.h>
#include <ecs/components/AnchorComponent.h>
#include <ecs/components/TextureComponent.h>

#include <string>

namespace ecs {
class EntityManager;
}

namespace gui::widgets {

class Widget {
public:
    static Widget createEntity(
        std::string name,
        ecs::EntityManager& manager);

    Widget& addPosition(int x, int y);
    Widget& addFill(graphics::core::Color color);
    Widget& addBorder(int size, graphics::core::Color color);
    Widget& addSize(int width, int height);
    Widget& addTexture(graphics::Texture id);

    Widget& anchorTop(
        ecs::Entity entity,
        ecs::VerticalPosition positionToAnchor,
        int margin = 0);

    Widget& anchorLeft(
        ecs::Entity entity,
        ecs::HorizontalPosition positionToAnchor,
        int margin = 0);

    Widget& anchorRight(
        ecs::Entity entity,
        ecs::HorizontalPosition positionToAnchor,
        int margin = 0);

    Widget& anchorBottom(
        ecs::Entity entity,
        ecs::VerticalPosition positionToAnchor,
        int margin = 0);

    const ecs::Entity entity;

private:
    Widget(ecs::Entity entity, ecs::EntityManager& manager);

    ecs::EntityManager& m_manager;
};

}
