#pragma once

#include <interfaces/ecs/Entity.h>

namespace graphics {
class GraphicsFactory;
}

namespace gui {

class MenuBar {
public:
    /// \brief Constructor
    /// \param entity_manager
    ///
    MenuBar(
        graphics::GraphicsFactory& entity_manager,
        ecs::Entity entity);
};

}
