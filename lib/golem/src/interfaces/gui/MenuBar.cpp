#include <interfaces/gui/MenuBar.h>

#include <interfaces/gui/style/Style.h>

#include <interfaces/graphics/GraphicsFactory.h>

namespace gui {

MenuBar::MenuBar(
    graphics::GraphicsFactory& graphics_factory,
    ecs::Entity entity)
{
    using Color = graphics::core::Color;
    auto new_button
        = graphics_factory
              .createImage(Style::imagePath(Style::Image::NewSong))
              .addSize(32, 32)
              .anchorLeft(entity, ecs::Left);

    auto open_button
        = graphics_factory
              .createImage(Style::imagePath(Style::Image::OpenSong))
              .addSize(32, 32)
              .anchorLeft(new_button.entity, ecs::Right, 1);

    auto save_button
        = graphics_factory
              .createImage(Style::imagePath(Style::Image::LoadSong))
              .addSize(32, 32)
              .anchorLeft(open_button.entity, ecs::Right, 1);

    auto export_button
        = graphics_factory
              .createImage(Style::imagePath(Style::Image::ExportSong))
              .addSize(32, 32)
              .anchorLeft(save_button.entity, ecs::Right, 1);
}

}
