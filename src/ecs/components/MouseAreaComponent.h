#pragma once

#include <ecs/Component.h>

#include <functional>

namespace ecs {

struct MouseArea {
    std::function<void()> onClicked;
    std::function<void()> onPressed;
    std::function<void()> onReleased;
};

using MouseAreaComponent = Component<MouseArea>;

}
