#pragma once

#include <ecs/Entity.h>

namespace ecs {
class EntityManager;
}

namespace gui {

class MenuBar {
public:
    MenuBar(
        ecs::EntityManager& entity_manager,
        ecs::Entity entity);
};

}
