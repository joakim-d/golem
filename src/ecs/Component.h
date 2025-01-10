#pragma once

namespace ecs {

template <typename Data>
struct Component {
    Data data;
    bool active;

    void reset()
    {
        active = false;
    }
};

}
