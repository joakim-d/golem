#pragma once

#include <interfaces/ecs/Component.h>

#include <interfaces/graphics/Painter.h>

#include <functional>

namespace ecs {
struct Draw {
    std::function<void(graphics::Painter& painter)> draw_callback;
};

using DrawComponent = Component<Draw>;

};
