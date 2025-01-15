#pragma once

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class SequencerView {
public:
    SequencerView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& sequencer_view_widget);

    void setOffsets(int x_offset, int y_offset);

    int viewWidth() const;

private:
    int m_x_offset;
    int m_y_offset;
};

}
