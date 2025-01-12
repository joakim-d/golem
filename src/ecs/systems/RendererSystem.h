#pragma once

#include <ecs/System.h>

#include <SDL_render.h>

namespace graphics {
class TextureManager;
}

namespace ecs {

class Renderer {
public:
    Renderer(
        SDL_Renderer* m_renderer,
        const graphics::TextureManager& font_manager);

    ~Renderer();

    void update(
        EntityMemoryPool& pool,
        const std::vector<Entity>& entities);

private:
    SDL_Renderer* m_renderer;
    const graphics::TextureManager& m_texture_manager;
};

}
