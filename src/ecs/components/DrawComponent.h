#pragma once

#include <ecs/Component.h>

#include <graphics/Painter.h>

#include <functional>

namespace ecs {
struct Draw {
    std::function<void(graphics::Painter& painter)> draw_callback;
};

using DrawComponent = Component<Draw>;

};
