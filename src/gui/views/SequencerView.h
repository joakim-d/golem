#pragma once

#include <memory>

namespace graphics {
class GraphicsFactory;
class Widget;
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
        std::shared_ptr<model::Song> song_model);

    void setOffsets(int x_offset, int y_offset);

    int viewWidth() const;

    void setCurrentTrackIndex(size_t track_index);

private:
    size_t m_current_track_index;
    int m_x_offset;
    int m_y_offset;
};

}
