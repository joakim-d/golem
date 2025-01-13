#pragma once

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class PianoRollView {
public:
    PianoRollView(
        graphics::GraphicsFactory& graphics_factory,
        const graphics::Widget& view_widget);
};
}
