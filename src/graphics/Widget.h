#pragma once

#include <SDL2/SDL.h>

#include <graphics/Painter.h>
#include <graphics/TextureManager.h>
#include <graphics/core/Color.h>

#include <ecs/Entity.h>
#include <ecs/components/AnchorComponent.h>
#include <ecs/components/KeyboardInputComponent.h>
#include <ecs/components/MouseAreaComponent.h>
#include <ecs/components/TextureComponent.h>

#include <string>

namespace ecs {
class EntityManager;
}

namespace graphics {

class Widget {
    friend class GraphicsFactory;
    friend class ImageFactory;
    friend class LabelFactory;

public:
    Widget& addPosition(int x, int y);
    Widget& addFill(graphics::core::Color color);
    Widget& addBorder(int size, graphics::core::Color color);
    Widget& addSize(int width, int height);
    Widget& addZPosition(int z);
    Widget& addTexture(graphics::Texture id);
    Widget& addUpdateCallback(std::function<void()> callback);

    Widget& onKeyPressed(
        std::function<void(ecs::KeyBoardContext)> callback);

    Widget& onPressed(
        ecs::Button accepted_buttons,
        std::function<void(ecs::Button, int x, int y)> callback);

    Widget& onReleased(
        ecs::Button accepted_buttons,
        std::function<void(ecs::Button, int x, int y)> callback);

    Widget& onClicked(
        ecs::Button accepted_buttons,
        std::function<void(ecs::Button, int x, int y)> callback);

    Widget& onPositionChanged(
        std::function<void(ecs::Button, int x, int y)> callback);

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

    Widget& fill(
        ecs::Entity entity,
        int margin = 0);

    Widget& onDraw(
        std::function<void(graphics::Painter& painter)> draw_callback);

    void die();

    int width() const;
    int height() const;
    int x() const;
    int y() const;
    int z() const;

    const ecs::Entity entity;

private:
    static Widget createEntity(
        std::string name,
        ecs::EntityManager& manager);

    Widget(ecs::Entity entity, ecs::EntityManager& manager);

    ecs::EntityManager& m_manager;
};

}
