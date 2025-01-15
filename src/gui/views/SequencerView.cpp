#include <gui/views/SequencerView.h>

#include <gui/views/PhraseView.h>
#include <gui/views/PianoKeyboardView.h>

#include <graphics/GraphicsFactory.h>

#include <iostream>

namespace {

static graphics::core::Color COLORS[12] {
    graphics::core::Color::fromHexa("#222222"),
    graphics::core::Color::fromHexa("#111111"),
    graphics::core::Color::fromHexa("#222222"),
    graphics::core::Color::fromHexa("#111111"),
    graphics::core::Color::fromHexa("#222222"),
    graphics::core::Color::fromHexa("#111111"),
    graphics::core::Color::fromHexa("#222222"),
    graphics::core::Color::fromHexa("#222222"),
    graphics::core::Color::fromHexa("#111111"),
    graphics::core::Color::fromHexa("#222222"),
    graphics::core::Color::fromHexa("#111111"),
    graphics::core::Color::fromHexa("#222222"),
};

constexpr int CELL_WIDTH = 16;
constexpr int SPACING = 1;
constexpr int NOTES_BY_PHRASE = 16;
constexpr int PHRASE_COUNT = 16;

}

namespace gui::views {

SequencerView::SequencerView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& sequencer_view_widget)
    : m_x_offset(0)
    , m_y_offset(0)
{
    sequencer_view_widget.onDraw(
        [sequencer_view_widget, this](graphics::Painter& painter) {
            const int cell_width = 16;
            const int cell_height = 13;
            const int cell_spacing = 1;
            const int cell_by_row = (sequencer_view_widget.width() / (cell_width + cell_spacing)) + 1;
            const int cell_by_column = (sequencer_view_widget.height() / (cell_height + cell_spacing)) + 1;
            const int start_j = m_y_offset / (cell_height + cell_spacing);
            const int start_i = m_x_offset / (cell_width + cell_spacing);

            const int draw_cells_on_row
                = start_i + cell_by_row > NOTES_BY_PHRASE * PHRASE_COUNT
                ? (16 * 16) - start_i
                : cell_by_row;

            const int draw_cells_on_column
                = start_j + cell_by_column > 12 * 6 // 12 notes * 6 octaves
                ? (12 * 6) - start_j
                : cell_by_column;

            const int start_x_offset = (m_x_offset % (cell_width + 1));
            const int start_y_offset = (m_y_offset % (cell_height + 1));

            SDL_Rect rects[cell_by_row * cell_by_column];
            int rect_index = 0;
            for (int j = 0; j < draw_cells_on_column; j++) {
                for (int i = 0; i < draw_cells_on_row; i++) {
                    rects[rect_index].w = cell_width;
                    rects[rect_index].h = cell_height;
                    rects[rect_index].x = i * (cell_width + cell_spacing) - start_x_offset;
                    rects[rect_index++].y = j * (cell_height + cell_spacing) - start_y_offset;
                }
                painter.fillRectangles(
                    rects + j * draw_cells_on_row,
                    draw_cells_on_row,
                    COLORS[(j + start_j) % 12]);
            }
        });
}

void SequencerView::setOffsets(int x_offset, int y_offset)
{
    m_x_offset = x_offset;
    m_y_offset = y_offset;
}

int SequencerView::viewWidth() const
{
    return (CELL_WIDTH + SPACING) * NOTES_BY_PHRASE * PHRASE_COUNT;
}

}
