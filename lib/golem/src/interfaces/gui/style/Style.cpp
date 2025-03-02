#include <interfaces/gui/style/Style.h>

namespace gui {

std::filesystem::path Style::fontPath()
{
    static std::filesystem::path font { "./resources/fonts/JetBrainsMono-Regular.ttf" };
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

std::filesystem::path Style::imagePath(Style::Image image_id)
{
    static std::filesystem::path image_paths[] = {
        "./resources/images/FileFile@2x.png",
        "./resources/images/Open@2x.png",
        "./resources/images/Save@2x.png",
        "./resources/images/Export@2x.png",
        "./resources/images/EffectDutyCycle@2x.png",
        "./resources/images/MenuDelete@4x.png"
    };

    return image_paths[(size_t)image_id];
}

}
