#include <gui/views/SongView.h>

#include <gui/style/Style.h>
#include <gui/views/PatternView.h>

#include <graphics/GraphicsFactory.h>
#include <graphics/Widget.h>

#include <memory>

namespace gui::views {

namespace {
    constexpr int Z_POSITION_OFFSET = 10;
}

struct MouseState {
    bool pressed { false };
    int previous_x { 0 };
    int x_offset { 0 };
};

SongView::SongView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& song_view_widget)
{
    const auto song_view_entity = song_view_widget.entity;
    const auto firstColor = graphics::core::Color::fromHexa("#111111");
    const auto secondColor = graphics::core::Color::fromHexa("#222222");

    std::vector<graphics::Widget> widgets_to_move;
    widgets_to_move.reserve(4 * 16);

    auto x_offset = std::make_shared<int>();

    // Header row
    auto header_first_cell
        = graphics_factory.createWidget("song_view_header_first_cell")
              .addFill(secondColor)
              .addSize(64, 16)
              .anchorTop(song_view_entity, ecs::Top)
              .anchorLeft(song_view_entity, ecs::Left)
              .addZPosition(Z_POSITION_OFFSET + 1);

    ecs::Entity previousEntity = header_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_header_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 16)
            .anchorTop(song_view_entity, ecs::Top)
            //.addPosition(65 + (128 * i), 0)
            .addUpdateCallback([current_widget, i, x_offset]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) + *x_offset, 0);
            });

        previousEntity = current_widget.entity;

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
              .anchorLeft(song_view_entity, ecs::Left)
              .addZPosition(Z_POSITION_OFFSET + 1);

    graphics_factory
        .createLabel("Pulse 1", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(pulse_first_cell.entity, ecs::Top, 16)
        .anchorLeft(pulse_first_cell.entity, ecs::Left, 8)
        .addZPosition(Z_POSITION_OFFSET + 2);

    previousEntity = pulse_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_header_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previousEntity, ecs::Top)
            .addPosition(65 + (128 * i), 0)
            .addUpdateCallback([current_widget, i, x_offset]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) + *x_offset, 0);
            });

        previousEntity = current_widget.entity;
    }

    // Pulse 2 row
    auto pulse_2_first_cell
        = graphics_factory.createWidget("song_view_pulse_2_first_cell")
              .addFill(secondColor)
              .addSize(64, 48)
              .anchorTop(pulse_first_cell.entity, ecs::Bottom, 1)
              .anchorLeft(song_view_entity, ecs::Left)
              .addZPosition(Z_POSITION_OFFSET + 1);

    graphics_factory
        .createLabel("Pulse 2", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(pulse_2_first_cell.entity, ecs::Top, 16)
        .anchorLeft(pulse_2_first_cell.entity, ecs::Left, 8)
        .addZPosition(Z_POSITION_OFFSET + 2);

    previousEntity = pulse_2_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_header_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previousEntity, ecs::Top)
            .addPosition(65 + (128 * i), 0)
            .addUpdateCallback([current_widget, i, x_offset]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) + *x_offset, 0);
            });

        previousEntity = current_widget.entity;
    }

    // Wave row
    auto wave_first_cell
        = graphics_factory.createWidget("song_view_wave_first_cell")
              .addFill(secondColor)
              .addSize(64, 48)
              .anchorTop(pulse_2_first_cell.entity, ecs::Bottom, 1)
              .anchorLeft(song_view_entity, ecs::Left)
              .addZPosition(1);

    graphics_factory
        .createLabel("Wave", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(wave_first_cell.entity, ecs::Top, 16)
        .anchorLeft(wave_first_cell.entity, ecs::Left, 8)
        .addZPosition(Z_POSITION_OFFSET + 2);

    previousEntity = wave_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_header_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previousEntity, ecs::Top)
            .addUpdateCallback([current_widget, i, x_offset]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) + *x_offset, 0);
            });

        previousEntity = current_widget.entity;
    }

    // Pulse 2 row
    auto noise_first_cell
        = graphics_factory.createWidget("song_view_noise_first_cell")
              .addFill(secondColor)
              .addSize(64, 48)
              .anchorTop(wave_first_cell.entity, ecs::Bottom, 1)
              .anchorLeft(song_view_entity, ecs::Left)
              .addZPosition(Z_POSITION_OFFSET + 1);

    graphics_factory
        .createLabel("Noise", 12, Style::fontPath(), Style::defaultColor())
        .anchorTop(noise_first_cell.entity, ecs::Top, 16)
        .anchorLeft(noise_first_cell.entity, ecs::Left, 8)
        .addZPosition(Z_POSITION_OFFSET + 2);

    previousEntity = noise_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_header_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previousEntity, ecs::Top)
            .addUpdateCallback([current_widget, i, x_offset]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) + *x_offset, 0);
            });

        views::PatternView pattern_view { graphics_factory, current_widget };

        previousEntity = current_widget.entity;
    }

    auto mouse_state = std::make_shared<MouseState>();

    song_view_widget.onPressed(
        ecs::Button::Middle,
        [mouse_state](ecs::Button, int x, int y) {
            mouse_state->pressed = true;
            mouse_state->previous_x = x;
        });

    song_view_widget.onReleased(
        ecs::Button::Middle,
        [mouse_state](ecs::Button, int x, int y) {
            mouse_state->pressed = false;
        });

    song_view_widget.onPositionChanged(
        [mouse_state, x_offset, song_view_widget](ecs::Button, int x, int y) {
            if (!mouse_state->pressed) {
                return;
            }
            const int delta_x = x - mouse_state->previous_x;
            *x_offset += delta_x;
            const auto max_delta = -128 * 16 + song_view_widget.width() - 64;
            if (*x_offset > 0) {
                *x_offset = 0;
            } else if (*x_offset < max_delta) {
                *x_offset = max_delta;
            }
            mouse_state->previous_x = x;
        });
}

}
