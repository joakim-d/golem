#pragma once

#include <graphics/Texture.h>

#include <filesystem>
#include <map>
#include <optional>
#include <vector>

namespace graphics {

class TextureManager;

class ImageManager {
public:
    ImageManager(TextureManager& texture_manager);

    std::optional<Texture> openImage(
        std::filesystem::path file_path);

private:
    TextureManager& m_texture_manager;
    std::map<std::filesystem::path, Texture> m_path_to_texture;
};

}
