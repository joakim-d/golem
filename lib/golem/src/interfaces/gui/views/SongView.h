#pragma once

#include <interfaces/graphics/Widget.h>

#include <interfaces/ecs/Entity.h>

#include <golem/utils/observable.h>

#include <memory>

namespace graphics
{
class GraphicsFactory;
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

class SongView
{
public:
    utils::observable<size_t> trackClicked;

    SongView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& song_view_widget,
        use_cases::ProjectUseCases& project_use_cases);

    void onProgressionChanged(size_t tick_index, size_t note_index);

private:
    int m_x_offset;
    graphics::Widget m_progression_bar;
};

}
