#pragma once

#include <interfaces/graphics/Widget.h>

namespace use_cases
{
class ProjectUseCases;
}

namespace core
{
class NotePlayer;
}

namespace graphics
{
class GraphicsFactory;
class Widget;
}

namespace gui::views
{

class PianoRollView
{
public:
    PianoRollView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        use_cases::ProjectUseCases& project_use_cases);

    graphics::Widget getWidget() const;

    void setYOffset(int y_offset);
    int viewHeight() const;

private:
    graphics::Widget m_view_widget;
    use_cases::ProjectUseCases& m_use_cases;
    int m_y_offset;
};
}
