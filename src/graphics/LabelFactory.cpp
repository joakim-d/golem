#include <graphics/LabelFactory.h>

#include <graphics/FontManager.h>
#include <graphics/TextureManager.h>
#include <graphics/Widget.h>

#include <ecs/EntityManager.h>
#include <ecs/components/TextureComponent.h>

#include <iostream>

#include <SDL2/SDL_ttf.h>

namespace graphics {

LabelFactory::LabelFactory(
    FontManager& font_manager,
    TextureManager& texture_manager,
    ecs::EntityManager& entity_manager)
    : m_font_manager(font_manager)
    , m_texture_manager(texture_manager)
    , m_entity_manager(entity_manager)
{
}

std::optional<Widget> LabelFactory::createLabel(
    const std::string& text,
    const std::filesystem::path& font_path,
    size_t point_size,
    graphics::core::Color color)
{
    auto font_id = m_font_manager.openFont(font_path, point_size);
    if (!font_id.has_value()) {
        std::cerr << "Failed to open font " << font_path << std::endl;
        SDL_Quit();
        return {};
    }

    auto font = m_font_manager.getFont(*font_id);
    SDL_Surface* surface = TTF_RenderText_Blended(
        font,
        text.c_str(),
        SDL_Color { color.r, color.g, color.b, color.a });

    auto texture = m_texture_manager.createFromSurface(surface);

    SDL_FreeSurface(surface);

    int width;
    int height;

    SDL_QueryTexture(
        m_texture_manager.getTexture(texture),
        nullptr,
        nullptr,
        &width,
        &height);

    auto entity
        = graphics::Widget::createEntity("label", m_entity_manager)
              .addSize(width, height)
              .addPosition(0, 0)
              .addTexture(texture);

    return entity;
}

}
