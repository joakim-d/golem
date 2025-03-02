#pragma once

#include <interfaces/ecs/Entity.h>

#include <memory>

namespace graphics
{
class GraphicsFactory;
class Widget;
}

namespace domain
{
class Song;
}

namespace use_cases
{
class ProjectUseCases;
}

namespace gui::views
{

class PatternView
{
public:
    PatternView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        use_cases::ProjectUseCases& project_use_cases,
        size_t track_index,
        size_t phrase_index);
};

}
