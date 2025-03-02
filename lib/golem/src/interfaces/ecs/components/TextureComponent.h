#pragma once

#include <interfaces/ecs/Component.h>

#include <cstddef>

namespace ecs {

using TextureId = size_t;

struct Texture {
    TextureId id;
};

using TextureComponent = Component<Texture>;

}
