#pragma once

#include <interfaces/ecs/Component.h>

namespace ecs {

struct Position {
    int x;
    int y;
};

struct ZPosition {
    int z;
};

using PositionComponent = Component<Position>;
using ZPositionComponent = Component<ZPosition>;

}
