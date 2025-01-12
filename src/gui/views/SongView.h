#pragma once

#include <ecs/Entity.h>

namespace graphics {
class GraphicsFactory;
}

namespace gui::views {

class SongView {
public:
    SongView(
        graphics::GraphicsFactory& graphics_factory,
        ecs::Entity song_view_entity);
};

}
