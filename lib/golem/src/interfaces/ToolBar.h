#pragma once

namespace use_cases
{
class ProjectUseCases;
}
namespace gui
{

class ToolBar
{
public:
    ToolBar(use_cases::ProjectUseCases& use_cases);

    void render();

private:
    use_cases::ProjectUseCases& m_use_cases;
};

}
