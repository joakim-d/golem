#pragma once

#include <ecs/Component.h>

#include <functional>

namespace ecs {

enum class Button {
    Left,
    Middle,
    Right
};

struct OnPressed {
    Button accepted_buttons;
    std::function<void(Button button)> on_pressed;
};

struct OnReleased {
    Button accepted_buttons;
    std::function<void(Button button)> on_released;
};

struct OnClicked {
    Button accepted_buttons;
    std::function<void(Button button)> on_clicked;
};

struct onPositionChanged {
    std::function<void(int x, int y)> on_position_changed;
};

using OnPressedComponent = Component<OnPressed>;
using OnReleasedComponent = Component<OnReleased>;
using OnClickedComponent = Component<OnClicked>;
using onPositionChangedComponent = Component<onPositionChanged>;

}
