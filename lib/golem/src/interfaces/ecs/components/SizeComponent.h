#pragma once

#include <interfaces/ecs/Component.h>

namespace ecs {

struct Size {
    int w;
    int h;
};

using SizeComponent = Component<Size>;

}
