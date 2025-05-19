#pragma once

#include <cstddef>

namespace use_cases
{

class ProjectUseCases;

}

namespace gui
{

class PatternView
{
public:
    PatternView(
        use_cases::ProjectUseCases& project_use_cases,
        size_t pattern_index);
};

}
