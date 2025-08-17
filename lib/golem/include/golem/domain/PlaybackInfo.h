#pragma once

#include <cstddef>

namespace domain
{

struct PlaybackInfo
{
    size_t current_pattern[4];
    size_t current_note { 0 };
    size_t current_tick { 0 };
    bool stopped { false };
    bool paused { false };
};

}
