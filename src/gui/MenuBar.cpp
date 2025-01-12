#include <gui/MenuBar.h>

#include <graphics/GraphicsFactory.h>

namespace gui {

MenuBar::MenuBar(
    graphics::GraphicsFactory& graphics_factory,
    ecs::Entity entity)
{
    using Color = graphics::core::Color;
    auto new_button
        = graphics_factory.createWidget("New Button")
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(entity, ecs::Left);

    auto open_button
        = graphics_factory.createWidget("Open Button")
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(new_button.entity, ecs::Right, 1);

    auto save_button
        = graphics_factory.createWidget("Save Button")
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(open_button.entity, ecs::Right, 1);

    auto export_button
        = graphics_factory.createWidget("Export Button")
              .addSize(32, 32)
              .addFill(Color { 255, 0, 0 })
              .anchorLeft(save_button.entity, ecs::Right, 1);
}

}
