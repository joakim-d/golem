#pragma once

#include <ecs/Entity.h>

namespace ecs {
class EntityManager;
}

namespace gui::views {

class PatternView {
    PatternView(
        ecs::EntityManager& entity_manager,
        ecs::Entity entity);
};

}
