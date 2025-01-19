#pragma once

#include <graphics/Texture.h>

#include <SDL2/SDL.h>

#include <vector>

namespace graphics {

class TextureManager {
public:
    TextureManager(SDL_Renderer* renderer);
    ~TextureManager();

    Texture createFromSurface(SDL_Surface* surface);

    SDL_Texture* getTexture(Texture id) const;

private:
    SDL_Renderer* m_renderer;
    std::vector<SDL_Texture*> m_textures;
};
}
