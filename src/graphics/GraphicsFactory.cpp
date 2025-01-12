#include <graphics/GraphicsFactory.h>

namespace graphics {

GraphicsFactory::GraphicsFactory(
    SDL_Renderer* renderer,
    ecs::EntityManager& entity_manager)
    : m_entity_manager(entity_manager)
    , m_texture_manager(renderer)
    , m_font_manager()
    , m_label_factory(m_font_manager, m_texture_manager, entity_manager)
{
}

Widget GraphicsFactory::createWidget(const std::string& name)
{
    return Widget::createEntity(name, m_entity_manager);
}

Widget GraphicsFactory::createLabel(
    const std::string& text,
    size_t point_size,
    const std::filesystem::path& font_path,
    graphics::core::Color color)
{
    return *m_label_factory.createLabel(
        text,
        font_path,
        point_size,
        color);
}

const TextureManager& GraphicsFactory::textureManager() const
{
    return m_texture_manager;
}

}
