#pragma once

#include <graphics/Widget.h>

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class PianoRollView {
public:
    PianoRollView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget);

    graphics::Widget getWidget() const;

    void setYOffset(int y_offset);
    int viewHeight() const;

private:
    graphics::Widget m_view_widget;
    int m_y_offset;
};
}
