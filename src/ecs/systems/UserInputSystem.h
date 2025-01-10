#pragma once

#include <ecs/System.h>

#include <SDL_render.h>

namespace ecs {

class UserInputSystem {
public:
    UserInputSystem(
        int window_width,
        int window_height);

    void update(
        EntityMemoryPool& pool,
        const std::vector<Entity>& entities);

private:
    void treatEvents();
    struct Mouse {
        int x;
        int y;
        enum ButtonStates {
            LeftPressed = 0x01,
            RightPressed = 0x02,
            MiddlePressed = 0x04
        };
        int button_states;
    };

    struct WindowSize {
        int width;
        int height;
    };

    WindowSize m_window_size;
    Mouse m_mouse;
};

}
