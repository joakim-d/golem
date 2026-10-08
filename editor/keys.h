#pragma once

#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_scancode.h>

#include <cstdint>
#include <optional>

namespace golem::editor {

// The character of the key at this position on a US QWERTY keyboard, for the tracker
// keyboard map (golem::edit::note_for_key): notes follow physical positions, whatever the
// layout (QWERTY, AZERTY, ...).
std::optional<char> qwerty_char(SDL_Scancode scancode);

// A typed hex digit: 0-9 from the number row or the keypad (by position, so AZERTY's number
// row works without Shift), A-F by letter (as the layout types them).
std::optional<std::uint8_t> hex_value(
    SDL_Scancode scancode,
    SDL_Keycode key);

} // namespace golem::editor
