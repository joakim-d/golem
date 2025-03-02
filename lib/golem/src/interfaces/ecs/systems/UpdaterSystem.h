#pragma once

#include <interfaces/ecs/System.h>

#include <SDL_render.h>

namespace ecs {

class Updater {
public:
    void update(
        EntityMemoryPool& pool,
        const std::vector<Entity>& entities);
};

}
