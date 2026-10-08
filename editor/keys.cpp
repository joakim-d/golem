#include "keys.h"

namespace golem::editor {

std::optional<char> qwerty_char(SDL_Scancode scancode)
{
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z) {
        return static_cast<char>('a' + (scancode - SDL_SCANCODE_A));
    }
    if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) {
        return static_cast<char>('1' + (scancode - SDL_SCANCODE_1));
    }
    switch (scancode) {
    case SDL_SCANCODE_0:
        return '0';
    case SDL_SCANCODE_COMMA:
        return ',';
    case SDL_SCANCODE_PERIOD:
        return '.';
    case SDL_SCANCODE_SEMICOLON:
        return ';';
    case SDL_SCANCODE_SLASH:
        return '/';
    default:
        return std::nullopt;
    }
}

std::optional<std::uint8_t> hex_value(
    SDL_Scancode scancode,
    SDL_Keycode key)
{
    if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9) {
        return static_cast<std::uint8_t>(1 + (scancode - SDL_SCANCODE_1));
    }
    if (scancode == SDL_SCANCODE_0 || scancode == SDL_SCANCODE_KP_0) {
        return 0;
    }
    if (scancode >= SDL_SCANCODE_KP_1 && scancode <= SDL_SCANCODE_KP_9) {
        return static_cast<std::uint8_t>(1 + (scancode - SDL_SCANCODE_KP_1));
    }
    if (key >= SDLK_A && key <= SDLK_F) {
        return static_cast<std::uint8_t>(10 + (key - SDLK_A));
    }
    return std::nullopt;
}

} // namespace golem::editor
