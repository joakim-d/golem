#include <graphics/Painter.h>

#include <SDL2/SDL.h>

namespace graphics {

Painter::Painter(
    SDL_Renderer* renderer,
    int x_offset,
    int y_offset)
    : m_renderer(renderer)
    , m_x_offset(x_offset)
    , m_y_offset(y_offset)
{
}

void Painter::drawRectangle(
    SDL_Rect& rectangle,
    graphics::core::Color color)
{
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(m_renderer, &rectangle);
}

void Painter::drawRectangles(
    SDL_Rect* rectangles,
    size_t count,
    graphics::core::Color color)
{
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRects(m_renderer, rectangles, count);
}

void Painter::fillRectangle(
    SDL_Rect& rectangle,
    graphics::core::Color color)
{
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(m_renderer, &rectangle);
}

void Painter::fillRectangles(
    SDL_Rect* rectangles,
    size_t count,
    graphics::core::Color color)
{
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRects(m_renderer, rectangles, count);
}

void Painter::drawLine(
    int x1,
    int y1,
    int x2,
    int y2,
    graphics::core::Color color)
{
    SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
}

}
