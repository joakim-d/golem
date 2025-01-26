#include <gui/views/SequencerView.h>

#include <gui/views/PhraseView.h>

#include <graphics/GraphicsFactory.h>

#include <model/Song.h>

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

constexpr int CELL_WIDTH = 15;
constexpr int CELL_HEIGHT = 13;
constexpr int CELL_SPACING = 1;
constexpr int NOTES_BY_PHRASE = 16;
constexpr int PHRASE_COUNT = 16;
constexpr int PHRASE_WIDTH = ((CELL_WIDTH + CELL_SPACING) * NOTES_BY_PHRASE);

}

namespace gui::views {

SequencerView::SequencerView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& sequencer_view_widget,
    graphics::Widget& progression_bar_widget,
    std::shared_ptr<model::Song> song_model)
    : m_sequencer_widget(sequencer_view_widget)
    , m_progression_bar_widget(progression_bar_widget)
    , m_song_model(song_model)
    , m_current_track_index(0)
    , m_x_offset(0)
    , m_y_offset(0)
    , m_last_note_index(0)
    , m_last_phrase_index(0)
{
    sequencer_view_widget.onDraw(
        [sequencer_view_widget, this, song_model](
            graphics::Painter& painter) {
            const int cell_by_row = (sequencer_view_widget.width() / (CELL_WIDTH + CELL_SPACING)) + 1;
            const int cell_by_column = (sequencer_view_widget.height() / (CELL_HEIGHT + CELL_SPACING)) + 1;
            const int start_j = m_y_offset / (CELL_HEIGHT + CELL_SPACING);
            const int start_i = m_x_offset / (CELL_WIDTH + CELL_SPACING);

            const int draw_cells_on_row
                = start_i + cell_by_row > NOTES_BY_PHRASE * PHRASE_COUNT
                ? (16 * 16) - start_i
                : cell_by_row;

            const int draw_cells_on_column
                = start_j + cell_by_column > 12 * 6 // 12 notes * 6 octaves
                ? (12 * 6) - start_j
                : cell_by_column;

            const int start_x_offset = (m_x_offset % (CELL_WIDTH + 1));
            const int start_y_offset = (m_y_offset % (CELL_HEIGHT + 1));

            // Draw note grid
            SDL_Rect rects[cell_by_row * cell_by_column];
            int rect_index = 0;
            for (int j = 0; j < draw_cells_on_column; j++) {
                for (int i = 0; i < draw_cells_on_row; i++) {
                    rects[rect_index].w = CELL_WIDTH;
                    rects[rect_index].h = CELL_HEIGHT;
                    rects[rect_index].x = i * (CELL_WIDTH + CELL_SPACING) - start_x_offset;
                    rects[rect_index++].y = j * (CELL_HEIGHT + CELL_SPACING) - start_y_offset;
                }
                painter.fillRectangles(
                    rects + j * draw_cells_on_row,
                    draw_cells_on_row,
                    COLORS[(j + start_j) % 12]);
            }

            // Draw notes
            SDL_Rect rect;
            rect.h = CELL_HEIGHT;
            for (int i = 0; i < draw_cells_on_row; i++) {
                const int current_phrase = (i + start_i) / NOTES_BY_PHRASE;
                const int current_note_index = (i + start_i) % NOTES_BY_PHRASE;

                auto phrase_index
                    = song_model->getTrack(
                                    m_current_track_index)
                          .phraseIndex(current_phrase);
                if (!phrase_index.has_value())
                    continue;

                const auto note
                    = song_model->getPhrase(
                                    *phrase_index)
                          .note(current_note_index);

                if (!note.has_value())
                    continue;

                rect.x = i * (CELL_WIDTH + CELL_SPACING) - start_x_offset;
                rect.y = ((6 * 12 - int(note->frequency())) - 1) * (CELL_HEIGHT + CELL_SPACING) - m_y_offset;
                rect.w = (CELL_WIDTH + CELL_SPACING) * note->duration();
                painter.fillRectangle(rect, graphics::core::Color::fromHexa("#ff0000"));
            }
        });
}

void SequencerView::setOffsets(int x_offset, int y_offset)
{
    m_x_offset = x_offset;
    m_y_offset = y_offset;
    updateProgressionBar();
}

int SequencerView::viewWidth() const
{
    return (CELL_WIDTH + CELL_SPACING) * NOTES_BY_PHRASE * PHRASE_COUNT;
}

void SequencerView::setCurrentTrackIndex(size_t track_index)
{
    m_current_track_index = track_index;
}

void SequencerView::onProgressionChanged(
    size_t tick_index,
    size_t note_index,
    size_t phrase_index)
{
    m_last_tick_index = tick_index;
    m_last_note_index = note_index;
    m_last_phrase_index = phrase_index;
    updateProgressionBar();
}

void SequencerView::updateProgressionBar()
{
    const int sequencer_widget_x = m_sequencer_widget.x();
    const int x_position = sequencer_widget_x
        + (double(m_last_tick_index) / double(m_song_model->ticksPerNote())) * CELL_WIDTH
        + m_last_phrase_index * PHRASE_WIDTH
        + (m_last_note_index * (CELL_WIDTH + CELL_SPACING))
        - m_x_offset;

    m_progression_bar_widget
        .addPosition(x_position >= sequencer_widget_x
                ? x_position
                : sequencer_widget_x,
            0);
}

}
