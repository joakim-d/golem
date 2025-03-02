#pragma once

#include <interfaces/gui/views/PianoRollView.h>
#include <interfaces/gui/views/SequencerHeaderView.h>
#include <interfaces/gui/views/SequencerView.h>

#include <memory>

namespace core
{
class NotePlayer;
}

namespace use_cases
{
class ProjectUseCases;
}

namespace gui::views
{

class MainView
{
public:
    MainView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        use_cases::ProjectUseCases& project_use_cases);

    void setCurrentTrackIndex(size_t track_index);

    void onProgressionChanged(size_t tick_index, size_t note_index);

private:
    PianoRollView m_piano_roll_view;
    SequencerHeaderView m_sequencer_header_view;
    SequencerView m_sequencer_view;
    int m_x_offset;
    int m_y_offset;
};

}
