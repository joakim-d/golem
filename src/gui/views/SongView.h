#pragma once

#include <graphics/Widget.h>

#include <ecs/Entity.h>

#include <utils/observable.h>

#include <memory>

namespace graphics {
class GraphicsFactory;
}

namespace model {
class Song;
}

namespace gui::views {

class SongView {
public:
    utils::observable<size_t> trackClicked;

    SongView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& song_view_widget,
        std::shared_ptr<model::Song> song_model);

    void onProgressionChanged(
        size_t tick_index,
        size_t note_index,
        size_t phrase_index);

private:
    int m_x_offset;
    graphics::Widget m_progression_bar;
    std::shared_ptr<model::Song> m_song_model;
};

}
