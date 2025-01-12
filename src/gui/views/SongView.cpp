#include <gui/views/SongView.h>

#include <gui/style/Style.h>

#include <graphics/GraphicsFactory.h>
#include <graphics/Widget.h>

namespace gui::views {

SongView::SongView(
    graphics::GraphicsFactory& graphics_factory,
    ecs::Entity song_view_entity)
{
    const auto firstColor = graphics::core::Color::fromHexa("#111111");
    const auto secondColor = graphics::core::Color::fromHexa("#222222");

    // Header row

    auto header_first_cell
        = graphics_factory.createWidget("song_view_header_first_cell")
              .addFill(secondColor)
              .addSize(64, 16)
              .anchorTop(song_view_entity, ecs::Top)
              .anchorLeft(song_view_entity, ecs::Left);

    ecs::Entity previousEntity = header_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        const auto currentWidget
            = graphics_factory.createWidget("song_view_header_cell")
                  .addFill(i % 2 == 0 ? firstColor : secondColor)
                  .addSize(128, 16)
                  .anchorTop(song_view_entity, ecs::Top)
                  .anchorLeft(previousEntity, ecs::Right, 1);

        previousEntity = currentWidget.entity;

        graphics_factory
            .createLabel(
                std::to_string(i + 1),
                12,
                Style::fontPath(),
                Style::defaultColor())
            .anchorTop(previousEntity, ecs::Top, 0)
            .anchorLeft(previousEntity, ecs::Left, 62);
    }

    // Pulse row
    auto pulse_first_cell
        = graphics_factory.createWidget("song_view_pulse_first_cell")
              .addFill(secondColor)
              .addSize(64, 48)
              .anchorTop(header_first_cell.entity, ecs::Bottom, 1)
              .anchorLeft(song_view_entity, ecs::Left);

    graphics_factory
        .createLabel("Pulse 1", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(pulse_first_cell.entity, ecs::Top, 16)
        .anchorLeft(pulse_first_cell.entity, ecs::Left, 8);

    previousEntity = pulse_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        const auto currentWidget
            = graphics_factory.createWidget("song_view_header_cell")
                  .addFill(i % 2 == 0 ? firstColor : secondColor)
                  .addSize(128, 48)
                  .anchorTop(previousEntity, ecs::Top)
                  .anchorLeft(previousEntity, ecs::Right, 1);

        previousEntity = currentWidget.entity;
    }

    // Pulse 2 row
    auto pulse_2_first_cell
        = graphics_factory.createWidget("song_view_pulse_2_first_cell")
              .addFill(secondColor)
              .addSize(64, 48)
              .anchorTop(pulse_first_cell.entity, ecs::Bottom, 1)
              .anchorLeft(song_view_entity, ecs::Left);

    graphics_factory
        .createLabel("Pulse 2", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(pulse_2_first_cell.entity, ecs::Top, 16)
        .anchorLeft(pulse_2_first_cell.entity, ecs::Left, 8);

    previousEntity = pulse_2_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        const auto currentWidget
            = graphics_factory.createWidget("song_view_header_cell")
                  .addFill(i % 2 == 0 ? firstColor : secondColor)
                  .addSize(128, 48)
                  .anchorTop(previousEntity, ecs::Top)
                  .anchorLeft(previousEntity, ecs::Right, 1);

        previousEntity = currentWidget.entity;
    }

    // Wave row
    auto wave_first_cell
        = graphics_factory.createWidget("song_view_wave_first_cell")
              .addFill(secondColor)
              .addSize(64, 48)
              .anchorTop(pulse_2_first_cell.entity, ecs::Bottom, 1)
              .anchorLeft(song_view_entity, ecs::Left);

    graphics_factory
        .createLabel("Wave", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(wave_first_cell.entity, ecs::Top, 16)
        .anchorLeft(wave_first_cell.entity, ecs::Left, 8);

    previousEntity = wave_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        const auto currentWidget
            = graphics_factory.createWidget("song_view_header_cell")
                  .addFill(i % 2 == 0 ? firstColor : secondColor)
                  .addSize(128, 48)
                  .anchorTop(previousEntity, ecs::Top)
                  .anchorLeft(previousEntity, ecs::Right, 1);

        previousEntity = currentWidget.entity;
    }

    // Pulse 2 row
    auto noise_first_cell
        = graphics_factory.createWidget("song_view_noise_first_cell")
              .addFill(secondColor)
              .addSize(64, 48)
              .anchorTop(wave_first_cell.entity, ecs::Bottom, 1)
              .anchorLeft(song_view_entity, ecs::Left);

    graphics_factory
        .createLabel("Noise", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(noise_first_cell.entity, ecs::Top, 16)
        .anchorLeft(noise_first_cell.entity, ecs::Left, 8);

    previousEntity = noise_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        const auto currentWidget
            = graphics_factory.createWidget("song_view_header_cell")
                  .addFill(i % 2 == 0 ? firstColor : secondColor)
                  .addSize(128, 48)
                  .anchorTop(previousEntity, ecs::Top)
                  .anchorLeft(previousEntity, ecs::Right, 1);

        previousEntity = currentWidget.entity;
    }
}

}
