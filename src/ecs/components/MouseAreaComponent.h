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
    std::function<void(Button button, int x, int y)> on_pressed;
};

struct OnReleased {
    Button accepted_buttons;
    std::function<void(Button button, int x, int y)> on_released;
};

struct OnClicked {
    Button accepted_buttons;
    std::function<void(Button button, int x, int y)> on_clicked;
};

struct onPositionChanged {
    std::function<void(Button button, int x, int y)> on_position_changed;
};

using OnPressedComponent = Component<OnPressed>;
using OnReleasedComponent = Component<OnReleased>;
using OnClickedComponent = Component<OnClicked>;
using OnPositionChangedComponent = Component<onPositionChanged>;

}
