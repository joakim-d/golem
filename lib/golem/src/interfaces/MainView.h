#pragma once

#include <cstddef>

namespace use_cases
{
class ProjectUseCases;
}

namespace gui
{

class MainView
{
public:
    MainView(
        use_cases::ProjectUseCases& project_use_cases,
        size_t song_index,
        size_t track_index);
};

}
