#pragma once

#include <ecs/Entity.h>

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class SongView {
public:
    SongView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& song_view_widget);
};

}
