#include "golem/notes.h"

#include <array>
#include <cmath>
#include <stdexcept>
#include <string>

namespace golem {

namespace {

    constexpr int kMidiOffset = 35; // Note 1 (C-2) is MIDI note 36.
    constexpr int kMaxNoiseShift = 13;
    constexpr int kMaxNoiseDivider = 7;

    void check_note(std::uint8_t note)
    {
        if (note < kFirstNote || note > kLastNote) {
            throw std::out_of_range("note out of range: " + std::to_string(note));
        }
    }

    double noise_frequency(
        int shift,
        int divider)
    {
        return 262144.0 / (divider == 0 ? 0.5 : divider) / std::pow(2.0, shift);
    }

    std::uint8_t nearest_noise(double frequency)
    {
        const double target = std::log2(frequency);
        std::uint8_t best = 0;
        double best_distance = INFINITY;
        // Shifts in increasing order with a strict comparison keeps the smallest shift on ties.
        for (int shift = 0; shift <= kMaxNoiseShift; ++shift) {
            for (int divider = 0; divider <= kMaxNoiseDivider; ++divider) {
                const double distance =
                    std::abs(std::log2(noise_frequency(shift, divider)) - target);
                if (distance < best_distance) {
                    best_distance = distance;
                    best = static_cast<std::uint8_t>(shift << 4 | divider);
                }
            }
        }
        return best;
    }

    template <
        typename T,
        typename F>
    std::array<
        T,
        kLastNote + 1>
    make_table(F f)
    {
        std::array<T, kLastNote + 1> table {};
        for (int note = kFirstNote; note <= kLastNote; ++note) {
            table[note] = f(static_cast<std::uint8_t>(note));
        }
        return table;
    }

} // namespace

double note_frequency(std::uint8_t note)
{
    check_note(note);
    return 440.0 * std::pow(2.0, (note + kMidiOffset - 69) / 12.0);
}

std::uint16_t note_period(std::uint8_t note)
{
    static const auto table = make_table<std::uint16_t>([](std::uint8_t n) {
        return static_cast<std::uint16_t>(std::lround(2048.0 - 131072.0 / note_frequency(n)));
    });
    check_note(note);
    return table[note];
}

std::uint8_t noise_nr43(std::uint8_t note)
{
    static const auto table =
        make_table<std::uint8_t>([](std::uint8_t n) { return nearest_noise(note_frequency(n)); });
    check_note(note);
    return table[note];
}

} // namespace golem
