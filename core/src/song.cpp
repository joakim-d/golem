#include "golem/song.h"

#include "golem/notes.h"

#include <algorithm>
#include <string>

namespace golem {

namespace {

    template <
        typename Instrument,
        std::size_t N>
    bool same_instruments(
        const std::array<
            Instrument,
            N>& a,
        const std::array<
            Instrument,
            N>& b)
    {
        return std::equal(
            a.begin(), a.end(), b.begin(), [](const Instrument& x, const Instrument& y) {
                return x.bytes == y.bytes;
            });
    }

} // namespace

bool operator==(
    const Cell& a,
    const Cell& b)
{
    return a.note == b.note
        && a.instrument == b.instrument
        && a.effect == b.effect
        && a.param == b.param;
}

bool operator!=(
    const Cell& a,
    const Cell& b)
{
    return !(a == b);
}

bool operator==(
    const Song& a,
    const Song& b)
{
    return a.ticks_per_row == b.ticks_per_row
        && a.orders == b.orders
        && a.patterns == b.patterns
        && same_instruments(a.pulse_instruments, b.pulse_instruments)
        && same_instruments(a.wave_instruments, b.wave_instruments)
        && same_instruments(a.noise_instruments, b.noise_instruments)
        && a.waves == b.waves;
}

bool operator!=(
    const Song& a,
    const Song& b)
{
    return !(a == b);
}

void validate_song(const Song& song)
{
    if (song.orders.empty() || song.orders.size() > kMaxOrders) {
        throw SongError(
            "order count must be 1.."
            + std::to_string(kMaxOrders)
            + ", got "
            + std::to_string(song.orders.size()));
    }
    for (std::size_t order = 0; order < song.orders.size(); ++order) {
        for (std::size_t channel = 0; channel < kChannels; ++channel) {
            if (song.orders[order][channel] >= song.patterns.size()) {
                throw SongError(
                    "order "
                    + std::to_string(order)
                    + " channel "
                    + std::to_string(channel + 1)
                    + ": no pattern "
                    + std::to_string(song.orders[order][channel]));
            }
        }
    }
    for (std::size_t pattern = 0; pattern < song.patterns.size(); ++pattern) {
        for (std::size_t row = 0; row < kRowsPerPattern; ++row) {
            const Cell& cell = song.patterns[pattern][row];
            const auto where = "pattern " + std::to_string(pattern) + " row " + std::to_string(row);
            if (cell.note > kLastNote) {
                throw SongError(where + ": note out of range: " + std::to_string(cell.note));
            }
            if (cell.instrument > kInstruments) {
                throw SongError(
                    where + ": instrument out of range: " + std::to_string(cell.instrument));
            }
            if (cell.effect > 0xF) {
                throw SongError(where + ": effect out of range: " + std::to_string(cell.effect));
            }
        }
    }
}

} // namespace golem
