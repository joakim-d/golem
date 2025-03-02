#pragma once

#include <cstddef>

namespace core
{

class IPlayingProgressionTracker
{
public:
    virtual ~IPlayingProgressionTracker() = default;
    virtual void onProgressMade(size_t tick_index, size_t note_index) = 0;
};

}
