#include <gui/views/SongView.h>

#include <gui/style/Style.h>
#include <gui/views/PatternView.h>

#include <graphics/GraphicsFactory.h>
#include <graphics/Widget.h>

#include <model/Song.h>

#include <memory>

namespace gui::views {

namespace {
    constexpr int Z_POSITION_OFFSET = 10;
    constexpr int PHRASE_WIDTH = 128;
    constexpr int NOTE_WIDTH = PHRASE_WIDTH / 16;
}

struct MouseState {
    bool pressed { false };
    int previous_x { 0 };
    int x_offset { 0 };
};

SongView::SongView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& song_view_widget,
    std::shared_ptr<model::Song> song_model)
    : m_progression_bar(graphics_factory.createWidget("progression_bar")
                            .anchorTop(song_view_widget.entity, ecs::Top)
                            .anchorBottom(song_view_widget.entity, ecs::Bottom)
                            .addSize(1, 0)
                            .addFill(graphics::core::Color { 255, 0, 0, 128 })
                            .addZPosition(1))
    , m_x_offset(0)
    , m_song_model(song_model)
{
    const auto song_view_entity = song_view_widget.entity;
    const auto firstColor = graphics::core::Color::fromHexa("#111111");
    const auto secondColor = graphics::core::Color::fromHexa("#222222");

    std::vector<graphics::Widget> widgets_to_move;
    widgets_to_move.reserve(4 * 16);

    // Header row
    auto header_first_cell
        = graphics_factory.createWidget("song_view_header_first_cell")
              .addFill(secondColor)
              .addSize(64, 16)
              .anchorTop(song_view_entity, ecs::Top)
              .anchorLeft(song_view_entity, ecs::Left)
              .addZPosition(Z_POSITION_OFFSET + 1);

    ecs::Entity previous_entity = header_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_header_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 16)
            .anchorTop(song_view_entity, ecs::Top)
            .addUpdateCallback([current_widget, i, this]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) - m_x_offset, 0);
            });

        previous_entity = current_widget.entity;

        graphics_factory
            .createLabel(
                std::to_string(i + 1),
                12,
                Style::fontPath(),
                Style::defaultColor())
            .anchorTop(previous_entity, ecs::Top, 0)
            .anchorLeft(previous_entity, ecs::Left, 62);
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

    previous_entity = pulse_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_pulse_1_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previous_entity, ecs::Top)
            .addPosition(65 + (128 * i), 0)
            .addUpdateCallback([current_widget, i, this]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) - m_x_offset, 0);
            })
            .onPressed(ecs::Button::Left, [this](ecs::Button, int x, int y) {
                trackClicked(0);
            });

        views::PatternView pattern_view {
            graphics_factory,
            current_widget,
            song_model,
            0,
            i
        };

        previous_entity = current_widget.entity;
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

    previous_entity = pulse_2_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_pulse_2_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previous_entity, ecs::Top)
            .addPosition(65 + (128 * i), 0)
            .addUpdateCallback([current_widget, i, this]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) - m_x_offset, 0);
            })
            .onPressed(ecs::Button::Left, [this](ecs::Button, int x, int y) {
                trackClicked(1);
            });

        views::PatternView pattern_view {
            graphics_factory,
            current_widget,
            song_model,
            1,
            i
        };

        previous_entity = current_widget.entity;
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

    previous_entity = wave_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_wave_first_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previous_entity, ecs::Top)
            .addUpdateCallback([current_widget, i, this]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) - m_x_offset, 0);
            })
            .onPressed(ecs::Button::Left, [this](ecs::Button, int x, int y) {
                trackClicked(2);
            });

        views::PatternView pattern_view {
            graphics_factory,
            current_widget,
            song_model,
            2,
            i
        };

        previous_entity = current_widget.entity;
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

    previous_entity = noise_first_cell.entity;

    for (size_t i = 0; i < 16; ++i) {
        auto current_widget
            = graphics_factory.createWidget("song_view_noise_cell");

        current_widget
            .addFill(i % 2 == 0 ? firstColor : secondColor)
            .addSize(128, 48)
            .anchorTop(previous_entity, ecs::Top)
            .addUpdateCallback([current_widget, i, this]() {
                auto widget = current_widget;
                widget.addPosition(65 + (128 * i) - m_x_offset, 0);
            })
            .onPressed(ecs::Button::Left, [this](ecs::Button, int x, int y) {
                trackClicked(3);
            });

        views::PatternView pattern_view {
            graphics_factory,
            current_widget,
            song_model,
            3,
            i
        };
        previous_entity = current_widget.entity;
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
        [mouse_state, this, song_view_widget](ecs::Button, int x, int y) {
            if (!mouse_state->pressed) {
                return;
            }
            const int delta_x = x - mouse_state->previous_x;
            m_x_offset -= delta_x;
            const auto max_delta = PHRASE_WIDTH * 16 - song_view_widget.width() + 64;

            if (m_x_offset < 0) {
                m_x_offset = 0;
            } else if (m_x_offset > max_delta) {
                m_x_offset = max_delta;
            }
            mouse_state->previous_x = x;
        });
}

void SongView::onProgressionChanged(
    size_t tick_index,
    size_t note_index,
    size_t phrase_index)
{
    m_progression_bar
        .addPosition(65
                + (double(tick_index) / double(m_song_model->ticksPerNote())) * NOTE_WIDTH
                + (phrase_index * PHRASE_WIDTH)
                + (NOTE_WIDTH * note_index) - m_x_offset,
            0);
}

}
