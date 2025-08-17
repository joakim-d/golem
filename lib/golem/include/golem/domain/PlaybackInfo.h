#pragma once

#include <cstddef>

namespace domain
{

struct PlaybackInfo
{
    size_t current_tick { 0 };
    size_t current_pattern[4];
    size_t current_note[4];
    bool paused[4];
    bool stopped;
};

}
