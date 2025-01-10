#include <ecs/systems/UserInputSystem.h>

#include <SDL2/SDL.h>
#include <ecs/EntityMemoryPool.h>

static bool containsPoint(SDL_Rect& rectangle, int point_x, int point_y)
{
    return (point_x >= rectangle.x && point_x < rectangle.x + rectangle.w)
        && (point_y >= rectangle.y && point_y < rectangle.y + rectangle.h);
}

namespace ecs {

UserInputSystem::UserInputSystem(
    int window_width,
    int window_height)
    : m_window_size({ window_width, window_height })
{
}

void UserInputSystem::update(
    EntityMemoryPool& pool,
    const std::vector<Entity>& entities)
{
    treatEvents();
    auto& window_entity_size = pool.getComponent<Size>(0);
    window_entity_size.w = m_window_size.width;
    window_entity_size.h = m_window_size.height;
    /*
        for (auto entity : entities) {
            auto& rectangle = pool.getComponent<Rectangle>(entity);
            if (containsPoint(rectangle.rectangle, m_mouse.x, m_mouse.y)) {
                rectangle.rectangle.x = m_mouse.x - rectangle.rectangle.w / 2;
                rectangle.rectangle.y = m_mouse.y - rectangle.rectangle.h / 2;
                if (m_mouse.button_states & Mouse::LeftPressed) {
                    pool.getComponent<MouseArea>(entity).onPressed();
                }
            }
        }*/
}

void UserInputSystem::treatEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_WINDOWEVENT:
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED) {
                m_window_size.width = event.window.data1;
                m_window_size.height = event.window.data2;
            }
            break;

        case SDL_MOUSEMOTION:
            m_mouse.x = event.motion.x;
            m_mouse.y = event.motion.y;
            break;

        case SDL_MOUSEBUTTONDOWN:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT:
                m_mouse.button_states |= Mouse::ButtonStates::LeftPressed;
                break;
            case SDL_BUTTON_RIGHT:
                m_mouse.button_states |= Mouse::ButtonStates::RightPressed;
                break;
            case SDL_BUTTON_MIDDLE:
                m_mouse.button_states |= Mouse::ButtonStates::MiddlePressed;
                break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT:
                m_mouse.button_states &= ~(Mouse::ButtonStates::LeftPressed);
                break;
            case SDL_BUTTON_RIGHT:
                m_mouse.button_states &= ~(Mouse::ButtonStates::RightPressed);
                break;
            case SDL_BUTTON_MIDDLE:
                m_mouse.button_states &= ~(Mouse::ButtonStates::MiddlePressed);
                break;
            }
            break;

        default:
            break;
        }
    }
}
}
