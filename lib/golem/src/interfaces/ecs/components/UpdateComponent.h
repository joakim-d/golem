#pragma once

#include <interfaces/ecs/Component.h>

#include <functional>

namespace ecs {

struct Update {
    std::function<void()> update;
};

using UpdateComponent = Component<Update>;

}
