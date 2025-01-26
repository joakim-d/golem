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
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                m_window_size.width = event.window.data1;
                m_window_size.height = event.window.data2;
            }
            break;

        case SDL_KEYDOWN:
            for (auto entity : m_last_entities_left_pressed) {
                if (pool.hasComponent<OnKeyPressedComponent>(entity)) {
                    ecs::KeyBoardContext context;
                    context.key_pressed = event.key.keysym.sym;
                    pool.getComponent<OnKeyPressed>(entity)
                        .on_pressed(context);
                }
            }
            break;

        case SDL_MOUSEMOTION:
            m_mouse.x = event.motion.x;
            m_mouse.y = event.motion.y;
            for (auto entity : m_entities_left_pressed) {
                if (pool.hasComponent<OnPositionChangedComponent>(entity)) {
                    const auto& position = pool.getComponent<Position>(entity);
                    pool.getComponent<OnPositionChanged>(entity)
                        .on_position_changed(
                            Button::Left,
                            m_mouse.x - position.x,
                            m_mouse.y - position.y);
                }
            }
            for (auto entity : m_entities_middle_pressed) {
                if (pool.hasComponent<OnPositionChangedComponent>(entity)) {
                    const auto& position = pool.getComponent<Position>(entity);
                    pool.getComponent<OnPositionChanged>(entity)
                        .on_position_changed(
                            Button::Middle,
                            m_mouse.x - position.x,
                            m_mouse.y - position.y);
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
                        if ((int)on_pressed.accepted_buttons & (int)Button::Left) {
                            const auto& position = pool.getComponent<Position>(entity);
                            on_pressed.on_pressed(
                                Button::Left,
                                m_mouse.x - position.x,
                                m_mouse.y - position.y);
                        }
                    }
                }
                m_last_entities_left_pressed = m_entities_left_pressed;
                break;
            case SDL_BUTTON_RIGHT:
                m_mouse.button_states |= Mouse::ButtonStates::RightPressed;
                break;
            case SDL_BUTTON_MIDDLE:
                lookForEntitiesPressed(m_entities_middle_pressed, entities, pool);
                for (auto entity : m_entities_middle_pressed) {
                    auto& on_released = pool.getComponent<OnPressed>(entity);
                    if ((int)on_released.accepted_buttons & (int)Button::Middle) {
                        const auto& position = pool.getComponent<Position>(entity);
                        on_released.on_pressed(
                            Button::Middle,
                            m_mouse.x - position.x,
                            m_mouse.y - position.y);
                        return;
                    }
                }
                break;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            switch (event.button.button) {
            case SDL_BUTTON_LEFT:
                for (auto entity : m_entities_left_pressed) {
                    auto& on_released = pool.getComponent<OnReleased>(entity);
                    if ((int)on_released.accepted_buttons & (int)Button::Left) {
                        const auto& position = pool.getComponent<Position>(entity);
                        on_released.on_released(
                            Button::Left,
                            m_mouse.x - position.x,
                            m_mouse.y - position.y);
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
                    auto& on_released = pool.getComponent<OnReleased>(entity);
                    if ((int)on_released.accepted_buttons & (int)Button::Middle) {
                        const auto& position = pool.getComponent<Position>(entity);
                        on_released.on_released(
                            Button::Middle,
                            m_mouse.x - position.x,
                            m_mouse.y - position.y);
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
    int previous_z = std::numeric_limits<int>::min();
    for (auto entity : entities) {
        const auto& position = pool.getComponent<Position>(entity);
        const auto& size = pool.getComponent<Size>(entity);
        SDL_Rect rectangle { position.x, position.y, size.w, size.h };
        if (!containsPoint(rectangle, m_mouse.x, m_mouse.y)) {
            continue;
        }
        entities_pressed.push_back(entity);
    }

    std::sort(
        entities_pressed.begin(),
        entities_pressed.end(),
        [&pool](Entity left, Entity right) {
            int z_left
                = pool.hasComponent<ZPositionComponent>(left)
                ? pool.getComponent<ZPosition>(left).z
                : 0;

            int z_right
                = pool.hasComponent<ZPositionComponent>(right)
                ? pool.getComponent<ZPosition>(right).z
                : 0;

            if (z_left == z_right) {
                return left > right;
            }

            return z_left > z_right;
        });
}
}
