#pragma once

#include <ecs/Entity.h>

namespace graphics {
class GraphicsFactory;
}

namespace gui {

class MenuBar {
public:
    MenuBar(
        graphics::GraphicsFactory& entity_manager,
        ecs::Entity entity);
};

}
