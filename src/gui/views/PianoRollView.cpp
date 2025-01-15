#include <gui/views/PianoKeyboardView.h>
#include <gui/views/PianoRollView.h>

#include <gui/views/PianoKeyboardView.h>

#include <graphics/GraphicsFactory.h>

#include <ecs/Entity.h>

namespace gui::views {

namespace {
    constexpr size_t OCTAVE_COUNT = 6;
    constexpr size_t NOTE_COUNT = 12;
    constexpr size_t WHITE_KEY_HEIGHT = 23;
    constexpr size_t BLACK_KEY_HEIGHT = 14;
    constexpr size_t SPACING = 1;

    static const bool IS_WHITE_LUT[12] {
        true,
        false,
        true,
        false,
        true,
        false,
        true,
        true,
        false,
        true,
        false,
        true
    };
}

PianoRollView::PianoRollView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget)
    : m_view_widget(view_widget)
    , m_y_offset(0)
{
    const int spacing = 1;
    const int white_key_height = 23;
    const int black_key_height = 14;
    const int white_key_width = 64;
    const int black_key_width = 46;

    SDL_Rect keys_rectangles[12 * 6];

    const int white_key_indexes[7] = { 0, 2, 4, 6, 7, 9, 11 };
    const int black_key_indexes[5] = { 1, 3, 5, 8, 10 };

    for (int octave = 0; octave < 6; octave++) {
        int i = 0;
        for (auto white_key_index : white_key_indexes) {
            keys_rectangles[octave * 12 + white_key_index].w = white_key_width;
            keys_rectangles[octave * 12 + white_key_index].h = white_key_height;
            keys_rectangles[octave * 12 + white_key_index].x = 0;
            keys_rectangles[octave * 12 + white_key_index].y = (168 * octave) + i * (white_key_height + 1);
            i++;
        }
        for (auto black_key_index : black_key_indexes) {
            keys_rectangles[octave * 12 + black_key_index].h = black_key_height;
            keys_rectangles[octave * 12 + black_key_index].w = black_key_width;
            keys_rectangles[octave * 12 + black_key_index].x = 0;
            keys_rectangles[octave * 12 + black_key_index].y = (168 * octave) + black_key_index * 14;
        }
    }

    view_widget.onDraw([keys_rectangles, view_widget, this](graphics::Painter& painter) {
        const int key_index_begin = (m_y_offset / (BLACK_KEY_HEIGHT));
        const int max_keys_to_draw = (view_widget.height() / BLACK_KEY_HEIGHT) + 1;
        const int keys_count_to_draw
            = key_index_begin + max_keys_to_draw > NOTE_COUNT * OCTAVE_COUNT
            ? NOTE_COUNT * OCTAVE_COUNT - key_index_begin
            : max_keys_to_draw;

        int index = 0;

        auto black_color = graphics::core::Color::fromHexa("#000000");
        auto white_color = graphics::core::Color::fromHexa("#ffffff");

        SDL_Rect current_key;

        for (int i = key_index_begin; i < key_index_begin + keys_count_to_draw; ++i) {
            if (!IS_WHITE_LUT[i % 12])
                continue;
            current_key.x = 0;
            current_key.y = keys_rectangles[i].y - m_y_offset;
            current_key.h = keys_rectangles[i].h;
            current_key.w = keys_rectangles[i].w;
            painter.fillRectangle(current_key, white_color);
        }

        for (int i = key_index_begin; i < key_index_begin + keys_count_to_draw; ++i) {
            if (IS_WHITE_LUT[i % 12])
                continue;
            current_key.x = 0;
            current_key.y = keys_rectangles[i].y - m_y_offset;
            current_key.h = keys_rectangles[i].h;
            current_key.w = keys_rectangles[i].w;
            painter.fillRectangle(current_key, black_color);
        }
    });
}

graphics::Widget PianoRollView::getWidget() const
{
    return m_view_widget;
}

void PianoRollView::setYOffset(int y_offset)
{
    m_y_offset = y_offset;
}

int PianoRollView::viewHeight() const
{
    return (WHITE_KEY_HEIGHT + SPACING) * 7 * OCTAVE_COUNT; // 7 white keys by octave * height
}

}
