#include <gui/MenuBar.h>

#include <ecs/EntityManager.h>

#include <gui/widgets/Widget.h>

namespace gui {

MenuBar::MenuBar(
    ecs::EntityManager& entity_manager,
    ecs::Entity entity)
{
    using Color = graphics::core::Color;
    auto new_button
        = gui::widgets::Widget::createEntity(
            "New Button",
            entity_manager)
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(entity, ecs::Left);

    auto open_button
        = gui::widgets::Widget::createEntity(
            "Open Button",
            entity_manager)
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(new_button.entity, ecs::Right, 1);

    auto save_button
        = gui::widgets::Widget::createEntity(
            "Save Button",
            entity_manager)
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(open_button.entity, ecs::Right, 1);

    auto export_button
        = gui::widgets::Widget::createEntity(
            "Export Button",
            entity_manager)
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(save_button.entity, ecs::Right, 1);
}

}
