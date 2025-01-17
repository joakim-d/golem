#pragma once

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class SequencerHeaderView {
public:
    SequencerHeaderView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& sequencer_view_widget);

    void setXOffset(int x_offset);

private:
    int m_x_offset;
};

}
