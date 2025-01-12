#pragma once

#include <ecs/Entity.h>

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class PianoKeyboardView {
public:
    PianoKeyboardView(
        graphics::GraphicsFactory& graphics_factory,
        const graphics::Widget& view_widget,
        unsigned octave);

private:
    unsigned m_octave;
};

}
