#pragma once

#include <interfaces/graphics/Widget.h>

#include <golem/domain/Note.h>

#include <memory>

namespace graphics
{
class GraphicsFactory;
}

namespace domain
{
class Song;
}

namespace use_cases
{
class ProjectUseCases;
}

namespace gui::views
{

class SequencerView
{
public:
    SequencerView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& sequencer_view_widget,
        graphics::Widget& progression_bar_widget,
        use_cases::ProjectUseCases& project_use_cases);

    void setOffsets(int x_offset, int y_offset);

    int viewWidth() const;

    void setCurrentTrackIndex(size_t track_index);

    void onProgressionChanged(size_t tick_index, size_t note_index);

private:
    void updateProgressionBar();

    graphics::Widget m_sequencer_widget;
    graphics::Widget m_progression_bar_widget;
    use_cases::ProjectUseCases& m_project_use_cases;
    size_t m_current_track_index;
    int m_x_offset;
    int m_y_offset;

    domain::NoteFrequency m_note_pressed_frequency;
    int m_note_pressed_index;
    int m_note_moving_index;
    bool m_note_edition;

    size_t m_progression_last_tick_index;
    size_t m_progression_last_note_index;
    size_t m_progression_last_phrase_index;
};

}
