#pragma once

#include <ecs/Entity.h>

#include <memory>

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace model {
class Song;
}

namespace gui::views {

class PatternView {
public:
    PatternView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        std::shared_ptr<model::Song> song_model,
        size_t track_index,
        size_t phrase_index);
};

}
