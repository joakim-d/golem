#pragma once

#include <ecs/Component.h>

#include <SDL2/SDL_rect.h>

namespace ecs {

struct Border {
    int size;
    SDL_Colour colour;
};

using BorderComponent = Component<Border>;

}
