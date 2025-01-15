#include <gui/views/MainView.h>

#include <graphics/GraphicsFactory.h>

namespace gui::views {

MainView::MainView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget)
    : m_piano_roll_view(
        graphics_factory,
        graphics_factory.createWidget("piano_roll")
            .addSize(64, 0)
            .anchorLeft(view_widget.entity, ecs::Left)
            .anchorTop(view_widget.entity, ecs::Top)
            .anchorBottom(view_widget.entity, ecs::Bottom))
    , m_sequencer_view(
          graphics_factory,
          graphics_factory.createWidget("sequencer")
              .anchorTop(view_widget.entity, ecs::Top)
              .anchorLeft(m_piano_roll_view.getWidget().entity, ecs::Right)
              .anchorRight(view_widget.entity, ecs::Right)
              .anchorBottom(view_widget.entity, ecs::Bottom))
    , m_x_offset(0)
    , m_y_offset(0)
{
    struct PressTracking {
        bool pressed;
        int previous_x;
        int previous_y;
    };

    auto press_tracking = std::make_shared<PressTracking>();
    view_widget.onPressed(
        ecs::Button::Middle,
        [press_tracking](ecs::Button, int x, int y) {
            press_tracking->pressed = true;
            press_tracking->previous_x = x;
            press_tracking->previous_y = y;
        });

    view_widget.onPositionChanged(
        [press_tracking, this, view_widget](ecs::Button, int x, int y) {
            if (!press_tracking->pressed) {
                return;
            }

            m_x_offset += press_tracking->previous_x - x;
            m_y_offset += press_tracking->previous_y - y;

            const int max_y_offset
                = m_piano_roll_view.viewHeight() - view_widget.height();

            const int max_x_offset
                = m_sequencer_view.viewWidth() - view_widget.width();

            if (m_y_offset > max_y_offset) {
                m_y_offset = max_y_offset;
            }

            if (m_x_offset > max_x_offset) {
                m_x_offset = max_x_offset;
            }

            if (m_x_offset < 0) {
                m_x_offset = 0;
            }
            if (m_y_offset < 0) {
                m_y_offset = 0;
            }

            press_tracking->previous_x = x;
            press_tracking->previous_y = y;

            m_sequencer_view.setOffsets(m_x_offset, m_y_offset);
            m_piano_roll_view.setYOffset(m_y_offset);
        });

    view_widget.onReleased(
        ecs::Button::Middle,
        [press_tracking](ecs::Button, int x, int y) {
            press_tracking->pressed = false;
        });
}

}
