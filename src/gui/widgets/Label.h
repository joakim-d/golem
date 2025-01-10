#pragma once

#include <SDL2/SDL.h>

#include <graphics/core/Color.h>

#include <ecs/Entity.h>
#include <ecs/components/AnchorComponent.h>

#include <string>

namespace ecs {
class EntityManager;
}

namespace graphics {
class LabelFactory;
}

namespace gui::widgets {

class Label {
public:
    static Label createEntity(
        std::string name,
        ecs::EntityManager& manager,
        graphics::LabelFactory& label_factory);
};

}
