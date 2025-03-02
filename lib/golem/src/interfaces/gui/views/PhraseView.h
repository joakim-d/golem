#pragma once

#include <interfaces/ecs/Entity.h>

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class PhraseView {
public:
    PhraseView(
        graphics::GraphicsFactory& graphics_factory,
        const graphics::Widget& view_widget);

    void moveView(
        int x_offset,
        int y_offset);

private:
    int x_offset;
    int y_offset;
};

}
