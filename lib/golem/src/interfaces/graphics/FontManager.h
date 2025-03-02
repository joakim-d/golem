#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <vector>

extern "C" {
struct _TTF_Font;
}

namespace graphics {

using Font = size_t;

class FontManager {
public:
    FontManager();
    ~FontManager();

    std::optional<Font> openFont(
        std::filesystem::path file_path,
        unsigned point_size);

    _TTF_Font* getFont(Font font_id) const;

private:
    struct FontKey {
        std::filesystem::path file_path;
        unsigned pointSize;
        bool operator<(const graphics::FontManager::FontKey& other) const;
    };
    std::vector<_TTF_Font*> m_fonts;
    std::map<FontKey, Font> m_info_to_font;
};

}
