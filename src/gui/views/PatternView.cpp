#include <gui/views/PatternView.h>

#include <graphics/Widget.h>

namespace gui::views {

PatternView::PatternView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget)
{
    const int width = view_widget.width();
    const int height = view_widget.height();

    view_widget.onDraw([width, height](graphics::Painter& painter) {
        SDL_Rect rectangle;

        for (int i = 0; i < 16; ++i) {
            rectangle.x = 10 * i;
            rectangle.y = 10 * i;
            rectangle.h = 10;
            rectangle.w = 10;

            painter.fillRectangle(rectangle, graphics::core::Color::fromName("red"));
        }
    });
}
}
