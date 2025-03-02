#include <interfaces/graphics/core/Color.h>

#include <map>

namespace graphics::core {

namespace {
    static const std::map<std::string, Color> NAMES_TO_COLORS {
        { "red", Color { 255, 0, 0 } },
        { "green", Color { 0, 255, 0 } },
        { "blue", Color { 0, 0, 255 } }
    };

    static const Color DEFAULT_COLOR = Color { 0, 0, 0 };
}

Color::Color(
    uint8_t r,
    uint8_t g,
    uint8_t b,
    uint8_t a)
    : r(r)
    , g(g)
    , b(b)
    , a(a)
{
}

Color Color::fromName(const std::string& name)
{
    const auto colorIterator = NAMES_TO_COLORS.find(name);
    if (colorIterator == NAMES_TO_COLORS.end()) {
        return Color { 0, 0, 0 };
    }
    return colorIterator->second;
}

Color Color::fromHexa(
    const std::string& hexadecimal_value)
{
    if (hexadecimal_value.size() != 7)
        return DEFAULT_COLOR;
    if (hexadecimal_value[0] != '#')
        return DEFAULT_COLOR;

    Color color;
    color.a = 255;

    unsigned int value
        = std::stoi(hexadecimal_value.substr(1), 0, 16);

    color.r = value >> 16;
    color.g = value >> 8;
    color.b = value;

    return color;
}

}
