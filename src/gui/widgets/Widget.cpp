#include <gui/widgets/Widget.h>

#include <ecs/EntityManager.h>

#include <ecs/components/AnchorComponent.h>
#include <ecs/components/BorderComponent.h>
#include <ecs/components/FillComponent.h>
#include <ecs/components/PositionComponent.h>
#include <ecs/components/SizeComponent.h>

namespace gui::widgets {

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
    widget.addPosition(0, 0);
    return widget;
}

Widget& Widget::addPosition(int x, int y)
{
    m_manager.addComponent<ecs::Position>(
        entity,
        ecs::Position { 0, 0 });

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

Widget& addBorder(int size, SDL_Color color);
Widget& Widget::addSize(int width, int height)
{
    m_manager.addComponent<ecs::Size>(
        entity,
        ecs::Size { width, height });

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

}
