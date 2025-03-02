#pragma once

#include <SDL_render.h>
#include <interfaces/graphics/core/Color.h>

namespace graphics {

class Painter {
public:
    Painter(
        SDL_Renderer* renderer,
        int x_offset,
        int y_offset);

    void drawRectangle(
        SDL_Rect& rectangle,
        graphics::core::Color color);

    void drawRectangles(
        SDL_Rect* rectangles,
        size_t count,
        graphics::core::Color color);

    void fillRectangle(
        SDL_Rect& rectangle,
        graphics::core::Color color);

    void fillRectangles(
        SDL_Rect* rectangles,
        size_t count,
        graphics::core::Color color);

    void drawLine(
        int x1,
        int y1,
        int x2,
        int y2,
        graphics::core::Color);

private:
    SDL_Renderer* m_renderer;
    int m_x_offset;
    int m_y_offset;
};

}
