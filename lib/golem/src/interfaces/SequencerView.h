#pragma once

#include <imgui.h>

namespace use_cases
{
class ProjectUseCases;
}

namespace gui
{

class SequencerView
{
public:
    SequencerView(use_cases::ProjectUseCases& use_cases, ImVec2 offset);

    static float viewWidth();
};

}
