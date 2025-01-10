#pragma once

#include <ecs/Component.h>

namespace ecs {

struct Size {
    int w;
    int h;
};

using SizeComponent = Component<Size>;

}
