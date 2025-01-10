#pragma once

#include <cstdint>
#include <string>

namespace graphics::core {

class Color {
public:
    Color() = default;
    Color(
        uint8_t r,
        uint8_t g,
        uint8_t b,
        uint8_t a = 255);

    static Color fromHexa(
        const std::string& hexadecimal_value);

    static Color fromName(
        const std::string& name);

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

}
