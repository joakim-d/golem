#include <interfaces/graphics/TextureManager.h>

#include <iostream>

namespace graphics {
TextureManager::TextureManager(SDL_Renderer* renderer)
    : m_renderer(renderer)
{
    m_textures.reserve(1024);
}

TextureManager::~TextureManager()
{
    for (auto texture : m_textures) {
        SDL_DestroyTexture(texture);
    }
}

Texture TextureManager::createFromSurface(SDL_Surface* surface)
{
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_renderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to open texture" << std::endl;
        SDL_Quit();
        return 0;
    }

    Texture id = m_textures.size();
    m_textures.push_back(texture);

    return id;
}

SDL_Texture* TextureManager::getTexture(Texture id) const
{
    return m_textures[id];
}

}
