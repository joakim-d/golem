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
            for (auto entity : m_entities_left_pressed) {
                if (pool.hasComponent<OnPositionChangedComponent>(entity)) {
                    pool.getComponent<onPositionChanged>(entity)
                        .on_position_changed(Button::Left, m_mouse.x, m_mouse.y);
                }
            }
            for (auto entity : m_entities_middle_pressed) {
                if (pool.hasComponent<OnPositionChangedComponent>(entity)) {
                    pool.getComponent<onPositionChanged>(entity)
                        .on_position_changed(Button::Middle, m_mouse.x, m_mouse.y);
                }
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT:
                lookForEntitiesPressed(m_entities_left_pressed, entities, pool);
                for (auto entity : m_entities_left_pressed) {
                    if (pool.hasComponent<OnPressedComponent>(entity)) {
                        auto& on_pressed = pool.getComponent<OnPressed>(entity);
                        if ((int)on_pressed.accepted_buttons & (int)Button::Left)
                            on_pressed.on_pressed(Button::Left, m_mouse.x, m_mouse.y);
                    }
                }
                break;
            case SDL_BUTTON_RIGHT:
                m_mouse.button_states |= Mouse::ButtonStates::RightPressed;
                break;
            case SDL_BUTTON_MIDDLE:
                lookForEntitiesPressed(m_entities_middle_pressed, entities, pool);
                for (auto entity : m_entities_middle_pressed) {
                    if (pool.hasComponent<OnPressedComponent>(entity)) {
                        pool.getComponent<OnPressed>(entity)
                            .on_pressed(Button::Middle, m_mouse.x, m_mouse.y);
                    }
                }
                break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT:
                for (auto entity : m_entities_left_pressed) {
                    if (pool.hasComponent<OnReleasedComponent>(entity)) {
                        pool.getComponent<OnReleased>(entity)
                            .on_released(Button::Left, m_mouse.x, m_mouse.y);
                    }
                }
                m_entities_left_pressed.clear();
                m_mouse.button_states &= ~(Mouse::ButtonStates::LeftPressed);
                break;
            case SDL_BUTTON_RIGHT:
                m_mouse.button_states &= ~(Mouse::ButtonStates::RightPressed);
                break;
            case SDL_BUTTON_MIDDLE:
                for (auto entity : m_entities_middle_pressed) {
                    if (pool.hasComponent<OnReleasedComponent>(entity)) {
                        pool.getComponent<OnReleased>(entity)
                            .on_released(Button::Middle, m_mouse.x, m_mouse.y);
                    }
                }
                m_entities_middle_pressed.clear();
                m_mouse.button_states &= ~(Mouse::ButtonStates::MiddlePressed);
                break;
            }
            break;

        default:
            break;
        }
    }

    auto& window_entity_size = pool.getComponent<Size>(0);
    window_entity_size.w = m_window_size.width;
    window_entity_size.h = m_window_size.height;
}

void UserInputSystem::lookForEntitiesPressed(
    std::vector<ecs::Entity>& entities_pressed,
    const std::vector<Entity>& entities,
    EntityMemoryPool& pool)
{
    ecs::Entity entity_pressed = 0;
    int previous_z = -10000;
    for (auto entity : entities) {
        const auto& position = pool.getComponent<Position>(entity);
        const auto& size = pool.getComponent<Size>(entity);
        SDL_Rect rectangle { position.x, position.y, size.w, size.h };
        if (!containsPoint(rectangle, m_mouse.x, m_mouse.y)) {
            continue;
        }
        entities_pressed.push_back(entity);
    }
}

}
