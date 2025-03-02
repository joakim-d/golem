#pragma once

#include <interfaces/graphics/core/Color.h>

#include <filesystem>

namespace gui {

class Style {
public:
    enum class Image {
        NewSong,
        OpenSong,
        LoadSong,
        ExportSong,
        Pulse,
        Close
    };

    static std::filesystem::path fontPath();
    static graphics::core::Color defaultColor();
    static size_t defaultTextSize();

    static std::filesystem::path imagePath(Image image_id);
};

}
