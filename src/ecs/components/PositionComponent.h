#pragma once

#include <ecs/Component.h>

namespace ecs {

struct Position {
    int x;
    int y;
};

using PositionComponent = Component<Position>;

}
