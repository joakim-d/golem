#pragma once

namespace use_cases
{
class ProjectUseCases;
}

namespace gui
{

class PianoRollView
{
public:
    PianoRollView(use_cases::ProjectUseCases& use_cases, float y_offset);

    static float viewHeight();
    static float viewWidth();
};

}
