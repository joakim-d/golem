#pragma once

#include <interfaces/ecs/Entity.h>

#include <interfaces/graphics/GraphicsFactory.h>
#include <interfaces/graphics/Widget.h>

#include <interfaces/gui/style/Style.h>
#include <golem/utils/observable.h>

namespace graphics {
class GraphicsFactory;
}

namespace gui {

template <typename View>
class ModalDialog {
public:
    ~ModalDialog()
    {
        m_modal_widget.die();
        m_close_icon.die();
    }

    template <typename... Args>
    static void spawn(
        graphics::GraphicsFactory& graphics_factory,
        Args&&... args)
    {
        new ModalDialog { graphics_factory, args... };
    }

private:
    static constexpr int Z_POSITION = 10000;
    static constexpr int CLOSE_ICON_MARGIN = 16;

    template <typename... Args>
    ModalDialog(
        graphics::GraphicsFactory& graphics_factory,
        Args&&... args)
        : m_modal_widget(
            graphics_factory
                .createWidget("modal_dialog")
                .addFill(graphics::core::Color::fromHexa("#111111"))
                .fill(0, 32)
                .addZPosition(Z_POSITION))
        , m_close_icon(
              graphics_factory
                  .createImage(
                      gui::Style::imagePath(gui::Style::Image::Close))
                  .anchorTop(m_modal_widget.entity, ecs::Top)
                  .anchorRight(m_modal_widget.entity, ecs::Right, CLOSE_ICON_MARGIN)
                  .addZPosition(Z_POSITION + 1)
                  .onPressed(ecs::Button::Left, [this](ecs::Button, int, int) {
                      delete this;
                  }))
        , m_content(std::forward<Args>(args)...)
    {
        m_content.fill(m_modal_widget.entity, 32);
        m_content.setZPosition(Z_POSITION + 2);
    }

    graphics::Widget m_modal_widget;
    graphics::Widget m_close_icon;
    View m_content;
};

}
