#pragma once

#include <cstddef>

namespace core {

class IPlayingProgressionTracker {
public:
    virtual ~IPlayingProgressionTracker() = default;
    virtual void onProgressMade(size_t note_index, size_t phrase_index) = 0;
};

}
