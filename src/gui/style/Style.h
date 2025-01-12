#pragma once

#include <graphics/core/Color.h>

#include <filesystem>

namespace gui {

class Style {
public:
    static std::filesystem::path fontPath();
    static graphics::core::Color defaultColor();
    static size_t defaultTextSize();
};

}
