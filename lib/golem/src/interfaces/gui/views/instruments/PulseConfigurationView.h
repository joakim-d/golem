#pragma once

#include <interfaces/graphics/Widget.h>
#include <memory>

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace domain {
class Song;
}

namespace gui::views {

class PulseConfigurationView {
public:
    PulseConfigurationView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        size_t instrument_index,
        std::shared_ptr<domain::Song>
            song_model);

    ~PulseConfigurationView();

    void fill(ecs::Entity entity, int margin);
    void setZPosition(int z_position);

private:
    graphics::Widget m_view_widget;
    graphics::Widget m_duty_cycle;
};

}
