#pragma once

#include <interfaces/ecs/System.h>

namespace ecs {

class AnchoringSystem {
public:
    void update(
        EntityMemoryPool& pool,
        const std::vector<Entity>& entities);
};

}
