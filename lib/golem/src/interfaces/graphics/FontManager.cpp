#include <interfaces/graphics/FontManager.h>

#include <SDL2/SDL_ttf.h>

#include <iostream>

namespace graphics {

bool FontManager::FontKey::operator<(const graphics::FontManager::FontKey& other) const
{
    return pointSize < other.pointSize && file_path < other.file_path;
}

FontManager::FontManager()
{
    if (TTF_Init() != 0) {
        std::cerr << "Failed to init TTF" << std::endl;
        SDL_Quit();
    }
    m_fonts.reserve(128);
}

FontManager::~FontManager()
{
    for (auto font : m_fonts) {
        TTF_CloseFont(font);
    }
}

std::optional<Font> FontManager::openFont(
    std::filesystem::path file_path, unsigned point_size)
{
    const auto font_iterator = m_info_to_font.find({ file_path, point_size });

    if (font_iterator != m_info_to_font.end()) {
        return font_iterator->second;
    }

    Font current_font = m_fonts.size();
    TTF_Font* ttf_font = TTF_OpenFont(file_path.c_str(), point_size);
    if (ttf_font == nullptr) {
        std::cerr << "Failed to open font: " << file_path << std::endl;
        return {};
    }

    m_fonts.push_back(ttf_font);

    return current_font;
}

_TTF_Font* FontManager::getFont(Font font_id) const
{
    return m_fonts[font_id];
}
}
