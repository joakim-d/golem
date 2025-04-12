#pragma once

#include <cstddef>

namespace use_cases
{
class ProjectUseCases;
}

namespace gui
{

class TrackView
{
public:
    TrackView(
        use_cases::ProjectUseCases& use_cases,
        size_t song_index,
        size_t track_index);
};

}
