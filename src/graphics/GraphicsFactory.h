#pragma once

#include <graphics/FontManager.h>
#include <graphics/LabelFactory.h>
#include <graphics/Widget.h>

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

    const graphics::TextureManager& textureManager() const;

private:
    ecs::EntityManager& m_entity_manager;
    graphics::TextureManager m_texture_manager;
    graphics::FontManager m_font_manager;
    graphics::LabelFactory m_label_factory;
};

}
