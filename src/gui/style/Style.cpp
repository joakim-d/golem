#include <gui/style/Style.h>

namespace gui {

std::filesystem::path Style::fontPath()
{
    static std::filesystem::path font { "/home/joachim/.local/share/fonts/JetBrainsMono-Regular.ttf" };
    return font;
}

graphics::core::Color Style::defaultColor()
{
    static auto color = graphics::core::Color::fromHexa("#eab2ff");
    return color;
}

size_t Style::defaultTextSize()
{
    return 12;
}

}
