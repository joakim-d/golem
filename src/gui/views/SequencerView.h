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
};

}
