#pragma once

#include <ecs/Entity.h>

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class PatternView {
public:
    PatternView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget);
};

}
