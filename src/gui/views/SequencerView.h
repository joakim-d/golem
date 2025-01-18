#pragma once

#include <graphics/Widget.h>

#include <memory>

namespace graphics {
class GraphicsFactory;
}

namespace model {
class Song;
}

namespace gui::views {

class SequencerView {
public:
    SequencerView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& sequencer_view_widget,
        graphics::Widget& progression_bar_widget,
        std::shared_ptr<model::Song> song_model);

    void setOffsets(int x_offset, int y_offset);

    int viewWidth() const;

    void setCurrentTrackIndex(size_t track_index);
    void onProgressionChanged(size_t note_index, size_t phrase_index);

private:
    void updateProgressionBar();
    graphics::Widget m_sequencer_widget;
    graphics::Widget m_progression_bar_widget;
    size_t m_current_track_index;
    int m_x_offset;
    int m_y_offset;
    size_t m_last_note_index;
    size_t m_last_phrase_index;
};

}
