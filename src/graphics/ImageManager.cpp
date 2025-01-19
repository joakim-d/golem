#include <graphics/ImageManager.h>

#include <graphics/TextureManager.h>

#include <SDL2/SDL_image.h>

#include <iostream>

namespace graphics {

ImageManager::ImageManager(TextureManager& texture_manager)
    : m_texture_manager(texture_manager)
{
}

std::optional<Texture> ImageManager::openImage(
    std::filesystem::path file_path)
{
    auto texture_iterator = m_path_to_texture.find(file_path);
    if (texture_iterator != m_path_to_texture.end()) {
        return texture_iterator->second;
    }

    auto surface = IMG_Load(file_path.c_str());
    if (surface == nullptr) {
        std::cerr << "Failed to open image: " << file_path.c_str() << std::endl;
        return {};
    }

    auto texture = m_texture_manager.createFromSurface(surface);
    SDL_FreeSurface(surface);

    m_path_to_texture[file_path] = texture;

    return texture;
}

}
