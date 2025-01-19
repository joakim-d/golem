#include <graphics/ImageFactory.h>

#include <graphics/ImageManager.h>

#include <iostream>

namespace graphics {

ImageFactory::ImageFactory(
    ImageManager& image_manager,
    TextureManager& texture_manager,
    ecs::EntityManager& entity_manager)
    : m_image_manager(image_manager)
    , m_texture_manager(texture_manager)
    , m_entity_manager(entity_manager)
{
}

std::optional<graphics::Widget> ImageFactory::createImage(
    std::filesystem::path path)
{
    auto texture = m_image_manager.openImage(path);
    if (!texture.has_value()) {
        std::cerr << "Failed to open image " << path << std::endl;
        SDL_Quit();
        return {};
    }

    int width;
    int height;

    SDL_QueryTexture(
        m_texture_manager.getTexture(*texture),
        nullptr,
        nullptr,
        &width,
        &height);

    auto widget
        = graphics::Widget::createEntity("image", m_entity_manager)
              .addSize(width, height)
              .addTexture(*texture);

    return widget;
}

}
