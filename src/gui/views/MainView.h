#pragma once

#include <gui/views/PianoRollView.h>
#include <gui/views/SequencerView.h>

namespace gui::views {

class MainView {
public:
    MainView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget);

private:
    PianoRollView m_piano_roll_view;
    SequencerView m_sequencer_view;
    int m_x_offset;
    int m_y_offset;
};

}
