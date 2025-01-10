#pragma once

#include <ecs/Component.h>

#include <SDL2/SDL_rect.h>

namespace ecs {

struct Fill {
    SDL_Colour colour;
};

using FillComponent = Component<Fill>;

}
