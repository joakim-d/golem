#pragma once

#include <ecs/Component.h>

#include <functional>

namespace ecs {

struct KeyBoardContext {
    char key_pressed;
};

struct OnKeyPressed {
    std::function<void(KeyBoardContext&)> on_pressed;
};

using OnKeyPressedComponent = Component<OnKeyPressed>;

}
