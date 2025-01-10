#include <gui/views/SongView.h>

#include <gui/widgets/Widget.h>

namespace gui::views {

SongView::SongView(
    ecs::EntityManager& entity_manager,
    ecs::Entity song_view_entity)
{
    const auto firstColor = graphics::core::Color::fromHexa("#111111");
    const auto secondColor = graphics::core::Color::fromHexa("#222222");

    auto header_first_cell
        = widgets::Widget::createEntity("song_view_header_first_cell", entity_manager)
              .addFill(secondColor)
              .addSize(64, 16)
              .anchorTop(song_view_entity, ecs::Top)
              .anchorLeft(song_view_entity, ecs::Left);

    ecs::Entity previousEntity = header_first_cell.entity;
    // header row
    for (size_t i = 0; i < 16; ++i) {
        const auto currentWidget
            = widgets::Widget::createEntity("song_view_header_cell", entity_manager)
                  .addFill(i % 2 == 0 ? firstColor : secondColor)
                  .addSize(128, 16)
                  .anchorTop(song_view_entity, ecs::Top)
                  .anchorLeft(previousEntity, ecs::Right, 1);

        previousEntity = currentWidget.entity;
    }
}

}
