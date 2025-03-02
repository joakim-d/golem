#pragma once

#include <interfaces/ecs/Component.h>
#include <interfaces/ecs/Entity.h>

#include <SDL2/SDL_rect.h>

namespace ecs {

enum VerticalPosition {
    Top,
    Bottom
};
enum HorizontalPosition {
    Left,
    Right
};

struct AnchorTop {
    Entity entity;
    VerticalPosition anchorTo;
    int margin;
};

struct AnchorLeft {
    Entity entity;
    HorizontalPosition anchorTo;
    int margin;
};

struct AnchorRight {
    Entity entity;
    HorizontalPosition anchorTo;
    int margin;
};

struct AnchorBottom {
    Entity entity;
    VerticalPosition anchorTo;
    int margin;
};

using AnchorTopComponent = Component<AnchorTop>;
using AnchorLeftComponent = Component<AnchorLeft>;
using AnchorRightComponent = Component<AnchorRight>;
using AnchorBottomComponent = Component<AnchorBottom>;

}
