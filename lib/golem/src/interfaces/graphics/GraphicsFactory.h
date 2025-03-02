#pragma once

#include <interfaces/graphics/FontManager.h>
#include <interfaces/graphics/ImageFactory.h>
#include <interfaces/graphics/ImageManager.h>
#include <interfaces/graphics/LabelFactory.h>
#include <interfaces/graphics/Widget.h>

namespace graphics {

class GraphicsFactory {
public:
    GraphicsFactory(
        SDL_Renderer* renderer,
        ecs::EntityManager& manager);

    GraphicsFactory(const GraphicsFactory& other) = delete;
    GraphicsFactory& operator=(const GraphicsFactory& other) = delete;

    Widget createWidget(
        const std::string& name);

    Widget createLabel(
        const std::string& text,
        size_t point_size,
        const std::filesystem::path& font_path,
        graphics::core::Color color);

    Widget createImage(
        std::filesystem::path image_path);

    const graphics::TextureManager& textureManager() const;

private:
    ecs::EntityManager& m_entity_manager;
    graphics::TextureManager m_texture_manager;
    graphics::FontManager m_font_manager;
    graphics::LabelFactory m_label_factory;
    graphics::ImageManager m_image_manager;
    graphics::ImageFactory m_image_factory;
};

}
