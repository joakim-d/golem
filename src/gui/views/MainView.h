#pragma once

#include <gui/views/PianoRollView.h>
#include <gui/views/SequencerHeaderView.h>
#include <gui/views/SequencerView.h>

#include <memory>

namespace core {
class NotePlayer;
}

namespace model {
class Song;
}

namespace gui::views {

class MainView {
public:
    MainView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        std::shared_ptr<model::Song>
            song_model,
        std::shared_ptr<core::NotePlayer>
            note_player);

    void setCurrentTrackIndex(size_t track_index);

    void onProgressionChanged(
        size_t tick_index,
        size_t phrase_index,
        size_t note_index);

private:
    PianoRollView m_piano_roll_view;
    SequencerHeaderView m_sequencer_header_view;
    SequencerView m_sequencer_view;
    int m_x_offset;
    int m_y_offset;
};

}
