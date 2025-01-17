#pragma once

#include <graphics/Widget.h>

#include <memory>

namespace core {
class NotePlayer;
}

namespace graphics {
class GraphicsFactory;
class Widget;
}

namespace gui::views {

class PianoRollView {
public:
    PianoRollView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        std::shared_ptr<core::NotePlayer>
            note_player);

    graphics::Widget getWidget() const;

    void setYOffset(int y_offset);
    int viewHeight() const;

private:
    graphics::Widget m_view_widget;
    std::shared_ptr<core::NotePlayer> m_note_player;
    int m_y_offset;
};
}
