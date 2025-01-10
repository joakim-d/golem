#pragma once

#include <ecs/Entity.h>
#include <ecs/components/TextureComponent.h>

#include <graphics/core/Color.h>

#include <filesystem>

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

    ecs::Entity createLabel(
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
