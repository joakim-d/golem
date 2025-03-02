#pragma once

#include <interfaces/graphics/Widget.h>

#include <filesystem>
#include <optional>

namespace ecs
{
class EntityManager;
}

namespace graphics
{
class ImageManager;
class TextureManager;

class ImageFactory
{
public:
    ImageFactory(
        ImageManager& image_manager,
        TextureManager& texture_manager,
        ecs::EntityManager& entity_manager);

    std::optional<graphics::Widget> createImage(std::filesystem::path path);

private:
    ImageManager& m_image_manager;
    TextureManager& m_texture_manager;
    ecs::EntityManager& m_entity_manager;
};

}
