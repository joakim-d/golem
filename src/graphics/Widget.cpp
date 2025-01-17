#include <graphics/Widget.h>

#include <ecs/EntityManager.h>

#include <ecs/components/AnchorComponent.h>
#include <ecs/components/BorderComponent.h>
#include <ecs/components/FillComponent.h>
#include <ecs/components/PositionComponent.h>
#include <ecs/components/SizeComponent.h>

namespace graphics {

Widget::Widget(ecs::Entity entity, ecs::EntityManager& manager)
    : entity(entity)
    , m_manager(manager)
{
}

Widget Widget::createEntity(
    std::string name,
    ecs::EntityManager& manager)
{
    auto entity = manager.addEntity(name);
    Widget widget { *entity, manager };
    widget.addPosition(0, 0)
        .addSize(0, 0);
    return widget;
}

Widget& Widget::addPosition(int x, int y)
{
    m_manager.addComponent<ecs::Position>(
        entity,
        ecs::Position { x, y });

    return *this;
}

Widget& Widget::addFill(graphics::core::Color color)
{
    m_manager.addComponent<ecs::Fill>(
        entity,
        ecs::Fill {
            SDL_Color { color.r, color.g, color.b, color.a } });
    return *this;
}

Widget& Widget::addBorder(int size, graphics::core::Color color)
{
    m_manager.addComponent<ecs::Border>(
        entity,
        ecs::Border { size, SDL_Color { color.r, color.g, color.b, color.a } });

    return *this;
}

Widget& Widget::addSize(int width, int height)
{
    m_manager.addComponent<ecs::Size>(
        entity,
        ecs::Size { width, height });

    return *this;
}

Widget& Widget::addZPosition(int z)
{
    m_manager.addComponent<ecs::ZPosition>(
        entity,
        ecs::ZPosition { z });

    return *this;
}

Widget& Widget::onKeyPressed(
    std::function<void(ecs::KeyBoardContext)> callback)
{
    m_manager.addComponent<ecs::OnKeyPressed>(
        entity,
        ecs::OnKeyPressed { std::move(callback) });

    return *this;
}

Widget& Widget::onPressed(
    ecs::Button accepted_buttons,
    std::function<void(ecs::Button, int x, int y)> callback)
{
    m_manager.addComponent<ecs::OnPressed>(
        entity,
        ecs::OnPressed { accepted_buttons, std::move(callback) });

    return *this;
}

Widget& Widget::onReleased(
    ecs::Button accepted_buttons,
    std::function<void(ecs::Button, int x, int y)> callback)
{
    m_manager.addComponent<ecs::OnReleased>(
        entity,
        ecs::OnReleased { accepted_buttons, std::move(callback) });

    return *this;
}

Widget& Widget::onClicked(
    ecs::Button accepted_buttons,
    std::function<void(ecs::Button, int x, int y)> callback)
{
    m_manager.addComponent<ecs::OnClicked>(
        entity,
        ecs::OnClicked { accepted_buttons, std::move(callback) });

    return *this;
}

Widget& Widget::onPositionChanged(
    std::function<void(ecs::Button, int x, int y)> callback)
{
    m_manager.addComponent<ecs::OnPositionChanged>(
        entity,
        ecs::OnPositionChanged { std::move(callback) });

    return *this;
}

Widget& Widget::anchorTop(
    ecs::Entity entity,
    ecs::VerticalPosition positionToAnchor,
    int margin)
{
    m_manager.addComponent<ecs::AnchorTop>(
        this->entity,
        ecs::AnchorTop { entity, positionToAnchor, margin });

    return *this;
}

Widget& Widget::anchorLeft(
    ecs::Entity entity,
    ecs::HorizontalPosition positionToAnchor,
    int margin)
{
    m_manager.addComponent<ecs::AnchorLeft>(
        this->entity,
        ecs::AnchorLeft { entity, positionToAnchor, margin });

    return *this;
}

Widget& Widget::anchorRight(
    ecs::Entity entity,
    ecs::HorizontalPosition positionToAnchor,
    int margin)
{
    m_manager.addComponent<ecs::AnchorRight>(
        this->entity,
        ecs::AnchorRight { entity, positionToAnchor, margin });

    return *this;
}

Widget& Widget::anchorBottom(
    ecs::Entity entity,
    ecs::VerticalPosition positionToAnchor,
    int margin)
{
    m_manager.addComponent<ecs::AnchorBottom>(
        this->entity,
        ecs::AnchorBottom { entity, positionToAnchor, margin });

    return *this;
}

Widget& Widget::addTexture(graphics::Texture texture)
{
    m_manager.addComponent<ecs::Texture>(
        entity,
        ecs::Texture { texture });

    return *this;
}

Widget& Widget::addUpdateCallback(std::function<void()> callback)
{
    m_manager.addComponent<ecs::Update>(
        entity,
        ecs::Update { std::move(callback) });

    return *this;
}

Widget& Widget::onDraw(std::function<void(graphics::Painter& painter)> draw_callback)
{
    m_manager.addComponent<ecs::Draw>(
        entity,
        ecs::Draw { std::move(draw_callback) });

    return *this;
}

int Widget::width() const
{
    return m_manager
        .getComponent<ecs::Size>(
            entity)
        .w;
}

int Widget::height() const
{
    return m_manager
        .getComponent<ecs::Size>(
            entity)
        .h;
}
int Widget::x() const
{
    return m_manager
        .getComponent<ecs::Position>(
            entity)
        .x;
}

int Widget::y() const
{
    return m_manager
        .getComponent<ecs::Position>(
            entity)
        .y;
}

}
