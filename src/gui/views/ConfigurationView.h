#pragma once

#include <graphics/Widget.h>

#include <utils/observable.h>

#include <memory>

namespace model {
class Song;
}

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class ConfigurationView {
public:
    utils::observable<size_t> configurePulseClicked;

    ConfigurationView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        std::shared_ptr<model::Song>
            song_model);
};

}
