#pragma once

#include <interfaces/ecs/Component.h>

#include <SDL2/SDL_rect.h>

namespace ecs {

struct Border {
    int size;
    SDL_Colour colour;
};

using BorderComponent = Component<Border>;

}
