#include <interfaces/gui/views/SequencerView.h>

#include <interfaces/gui/views/PhraseView.h>

#include <interfaces/graphics/GraphicsFactory.h>

#include <golem/domain/Song.h>
#include <golem/use_cases/ProjectUseCases.h>

namespace
{

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

namespace gui::views
{

SequencerView::SequencerView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& sequencer_view_widget,
    graphics::Widget& progression_bar_widget,
    use_cases::ProjectUseCases& project_use_cases)
    : m_sequencer_widget(sequencer_view_widget)
    , m_progression_bar_widget(progression_bar_widget)
    , m_project_use_cases(project_use_cases)
    , m_current_track_index(0)
    , m_x_offset(0)
    , m_y_offset(0)
    , m_note_edition(false)
    , m_progression_last_note_index(0)
    , m_progression_last_phrase_index(0)
{
    sequencer_view_widget.onDraw(
        [sequencer_view_widget, this](graphics::Painter& painter)
        {
            const int cell_by_row
                = (sequencer_view_widget.width() / (CELL_WIDTH + CELL_SPACING))
                + 1;
            const int cell_by_column = (sequencer_view_widget.height()
                                        / (CELL_HEIGHT + CELL_SPACING))
                + 1;
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
            for (int j = 0; j < draw_cells_on_column; j++)
            {
                for (int i = 0; i < draw_cells_on_row; i++)
                {
                    rects[rect_index].w = CELL_WIDTH;
                    rects[rect_index].h = CELL_HEIGHT;
                    rects[rect_index].x
                        = i * (CELL_WIDTH + CELL_SPACING) - start_x_offset;
                    rects[rect_index++].y
                        = j * (CELL_HEIGHT + CELL_SPACING) - start_y_offset;
                }
                painter.fillRectangles(
                    rects + j * draw_cells_on_row, draw_cells_on_row,
                    COLORS[(j + start_j) % 12]);
            }

            auto note_color = m_note_edition
                ? graphics::core::Color { 255, 0, 0, 128 }
                : graphics::core::Color::fromHexa("#ff0000");

            // Draw notes
            SDL_Rect rect;
            rect.h = CELL_HEIGHT;

            const size_t notes_per_phrase
                = m_project_use_cases.get_notes_per_phrase.execute(0);

            const auto track = m_project_use_cases.get_track.execute(
                0, m_current_track_index);

            if (!track)
            {
                return;
            }

            for (int i = 0; i < draw_cells_on_row; i++)
            {
                const auto note = track->note(i + start_i);

                if (!note.has_value())
                    continue;

                rect.x = i * (CELL_WIDTH + CELL_SPACING) - start_x_offset + 1;
                rect.y = ((6 * 12 - int(note->frequency())) - 1)
                        * (CELL_HEIGHT + CELL_SPACING)
                    - m_y_offset + 1;
                rect.w = (CELL_WIDTH + CELL_SPACING) * note->duration() - 2;
                painter.fillRectangle(rect, note_color);
            }

            // Draw inserted note, if any
            if (!m_note_edition)
            {
                return;
            }

            const int absolute_note_index
                = std::min(m_note_pressed_index, m_note_moving_index);

            rect.x = absolute_note_index * (CELL_WIDTH + CELL_SPACING)
                - m_x_offset + 1;

            rect.y = ((6 * 12 - int(m_note_pressed_frequency)) - 1)
                    * (CELL_HEIGHT + CELL_SPACING)
                - m_y_offset + 1;

            rect.w = (CELL_WIDTH + CELL_SPACING)
                    * (std::abs(m_note_moving_index - m_note_pressed_index) + 1)
                - 2;

            painter.fillRectangle(
                rect, graphics::core::Color::fromHexa("#ff0000"));
        });

    sequencer_view_widget.onPressed(
        ecs::Button::Left,
        [this](ecs::Button, int x, int y)
        {
            m_note_edition = true;

            m_note_pressed_index
                = (x + m_x_offset) / (CELL_WIDTH + CELL_SPACING);

            m_note_moving_index = m_note_pressed_index;

            m_note_pressed_frequency = static_cast<domain::NoteFrequency>(
                (6 * 12) - (y + m_y_offset) / (CELL_HEIGHT + CELL_SPACING) - 1);

            m_project_use_cases.play_note.execute(
                0, static_cast<domain::Channel>(m_current_track_index),
                m_note_pressed_frequency);
        });

    sequencer_view_widget.onPositionChanged(
        [this](ecs::Button, int x, int y) {
            m_note_moving_index
                = (x + m_x_offset) / (CELL_WIDTH + CELL_SPACING);
        });

    sequencer_view_widget.onReleased(
        ecs::Button::Left,
        [this](ecs::Button, int x, int y)
        {
            m_note_edition = false;

            const int note_released_index
                = (x + m_x_offset) / (CELL_WIDTH + CELL_SPACING);

            const unsigned note_duration
                = std::abs(m_note_pressed_index - note_released_index) + 1;

            const int absolute_note_index
                = std::min(m_note_pressed_index, note_released_index);

            m_project_use_cases.stop_note.execute(
                static_cast<domain::Channel>(m_current_track_index));

            m_project_use_cases.add_note.execute(
                0, m_current_track_index, absolute_note_index,
                domain::Note { m_note_pressed_frequency, note_duration, 0 });
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

void SequencerView::onProgressionChanged(size_t tick_index, size_t note_index)
{
    m_progression_last_tick_index = tick_index;
    m_progression_last_note_index = note_index;
    updateProgressionBar();
}

void SequencerView::updateProgressionBar()
{
    constexpr double ticks_per_note = 16.;

    const int sequencer_widget_x = m_sequencer_widget.x();
    const int x_position = sequencer_widget_x
        + (double(m_progression_last_tick_index) / ticks_per_note) * CELL_WIDTH
        + m_progression_last_phrase_index * PHRASE_WIDTH
        + (m_progression_last_note_index * (CELL_WIDTH + CELL_SPACING))
        - m_x_offset;

    m_progression_bar_widget.addPosition(
        x_position >= sequencer_widget_x ? x_position : sequencer_widget_x, 0);
}

}
