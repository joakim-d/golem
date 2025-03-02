#pragma once

#include <interfaces/graphics/Widget.h>
#include <interfaces/graphics/core/Color.h>

#include <filesystem>
#include <optional>

namespace ecs {
class EntityManager;
}

namespace graphics {

class FontManager;
class TextureManager;

class LabelFactory {
public:
    LabelFactory(
        FontManager& font_manager,
        TextureManager& texture_manager,
        ecs::EntityManager& entity_manager);

    std::optional<Widget> createLabel(
        const std::string& text,
        const std::filesystem::path& font_path,
        size_t point_size,
        graphics::core::Color color);

private:
    FontManager& m_font_manager;
    TextureManager& m_texture_manager;
    ecs::EntityManager& m_entity_manager;
};

}
