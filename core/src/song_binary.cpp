#include "golem/song_binary.h"

#include <map>
#include <string>

namespace golem {

namespace {

    constexpr std::size_t kHeaderSize = 19;
    constexpr std::size_t kCellSize = 3;
    constexpr std::size_t kPatternSize = kRowsPerPattern * kCellSize;
    constexpr std::size_t kPulseSize = 3;
    constexpr std::size_t kWaveInstrumentSize = 2;
    constexpr std::size_t kNoiseSize = 2;
    constexpr std::size_t kAddressSpace = 0x10000;

    class Writer {
    public:
        explicit Writer(std::uint16_t base)
            : base_(base)
        {
        }

        std::uint16_t address() const
        {
            return static_cast<std::uint16_t>(base_ + bytes_.size());
        }

        void byte(std::uint8_t value)
        {
            bytes_.push_back(value);
        }

        void word(std::uint16_t value)
        {
            byte(static_cast<std::uint8_t>(value & 0xFF));
            byte(static_cast<std::uint8_t>(value >> 8));
        }

        void patch_word(
            std::size_t offset,
            std::uint16_t value)
        {
            bytes_[offset] = static_cast<std::uint8_t>(value & 0xFF);
            bytes_[offset + 1] = static_cast<std::uint8_t>(value >> 8);
        }

        template <std::size_t N>
        void bytes(
            const std::array<
                std::uint8_t,
                N>& values)
        {
            bytes_.insert(bytes_.end(), values.begin(), values.end());
        }

        std::vector<std::uint8_t> take()
        {
            return std::move(bytes_);
        }

    private:
        std::uint16_t base_;
        std::vector<std::uint8_t> bytes_;
    };

    class Reader {
    public:
        Reader(
            const std::vector<std::uint8_t>& bytes,
            std::uint16_t base)
            : bytes_(bytes)
            , base_(base)
        {
        }

        // Offset of `size` bytes at `address`; throws if they are not all inside the data.
        std::size_t offset(
            std::uint16_t address,
            std::size_t size,
            const char* what) const
        {
            if (address < base_ || address - base_ + size > bytes_.size()) {
                throw SongError(
                    std::string(what) + " at $" + hex(address) + " is outside the song data");
            }
            return address - base_;
        }

        std::uint8_t byte_at(std::size_t offset) const
        {
            return bytes_[offset];
        }

        std::uint16_t word_at(std::size_t offset) const
        {
            return static_cast<std::uint16_t>(bytes_[offset] | bytes_[offset + 1] << 8);
        }

        template <std::size_t N>
        void read(
            std::uint16_t address,
            std::array<
                std::uint8_t,
                N>& values,
            const char* what) const
        {
            const auto start = offset(address, N, what);
            // A plain loop: std::copy_n here trips a GCC 13 -Wstringop-overflow false positive.
            for (std::size_t i = 0; i < N; ++i) {
                values[i] = bytes_[start + i];
            }
        }

    private:
        static std::string hex(std::uint16_t value)
        {
            static const char* digits = "0123456789ABCDEF";
            std::string text(4, '0');
            for (int i = 3; i >= 0; --i, value >>= 4) {
                text[i] = digits[value & 0xF];
            }
            return text;
        }

        const std::vector<std::uint8_t>& bytes_;
        std::uint16_t base_;
    };

} // namespace

std::vector<std::uint8_t> encode_song(
    const Song& song,
    std::uint16_t base)
{
    validate_song(song);

    const std::size_t size = kHeaderSize
                           + 1
                           + kChannels * 2 * song.orders.size()
                           + kInstruments * (kPulseSize + kWaveInstrumentSize + kNoiseSize)
                           + kWaves * kWaveBytes
                           + song.patterns.size() * kPatternSize;
    if (base + size > kAddressSpace) {
        throw SongError("song of " + std::to_string(size) + " bytes does not fit at this base");
    }

    Writer out(base);
    out.byte(song.ticks_per_row);
    for (std::size_t i = 0; i < (kHeaderSize - 1) / 2; ++i) {
        out.word(0); // Header pointers, patched below.
    }
    std::size_t pointer = 1;
    auto patch_next = [&](std::uint16_t address) {
        out.patch_word(pointer, address);
        pointer += 2;
    };

    patch_next(out.address());
    out.byte(static_cast<std::uint8_t>(song.orders.size()));

    const std::uint16_t first_pattern =
        static_cast<std::uint16_t>(base + size - song.patterns.size() * kPatternSize);
    for (std::size_t channel = 0; channel < kChannels; ++channel) {
        patch_next(out.address());
        for (const auto& order : song.orders) {
            out.word(static_cast<std::uint16_t>(first_pattern + order[channel] * kPatternSize));
        }
    }

    patch_next(out.address());
    for (const auto& instrument : song.pulse_instruments) {
        out.bytes(instrument.bytes);
    }
    patch_next(out.address());
    for (const auto& instrument : song.wave_instruments) {
        out.bytes(instrument.bytes);
    }
    patch_next(out.address());
    for (const auto& instrument : song.noise_instruments) {
        out.bytes(instrument.bytes);
    }
    patch_next(out.address());
    for (const auto& wave : song.waves) {
        out.bytes(wave);
    }

    for (const auto& pattern : song.patterns) {
        for (const auto& cell : pattern) {
            out.byte(cell.note);
            out.byte(static_cast<std::uint8_t>(cell.instrument << 4 | cell.effect));
            out.byte(cell.param);
        }
    }
    return out.take();
}

Song decode_song(
    const std::vector<std::uint8_t>& bytes,
    std::uint16_t base)
{
    if (bytes.size() < kHeaderSize) {
        throw SongError("song data shorter than the header");
    }
    const Reader in(bytes, base);
    Song song;
    song.ticks_per_row = in.byte_at(0);

    const auto order_count = in.byte_at(in.offset(in.word_at(1), 1, "order count"));
    if (order_count == 0) {
        throw SongError("order count is 0");
    }
    song.orders.resize(order_count);

    std::map<std::uint16_t, std::uint8_t> pattern_indices; // Pattern address -> index.
    std::vector<std::uint16_t> pattern_addresses;
    std::array<std::size_t, kChannels> tables;
    for (std::size_t channel = 0; channel < kChannels; ++channel) {
        tables[channel] = in.offset(in.word_at(3 + 2 * channel), 2 * order_count, "order table");
    }
    for (std::size_t order = 0; order < order_count; ++order) {
        for (std::size_t channel = 0; channel < kChannels; ++channel) {
            const auto address = in.word_at(tables[channel] + 2 * order);
            auto [it, inserted] = pattern_indices.emplace(
                address, static_cast<std::uint8_t>(pattern_addresses.size()));
            if (inserted) {
                pattern_addresses.push_back(address);
            }
            song.orders[order][channel] = it->second;
        }
    }

    const auto pulse = in.word_at(11);
    for (std::size_t i = 0; i < kInstruments; ++i) {
        in.read(
            static_cast<std::uint16_t>(pulse + i * kPulseSize),
            song.pulse_instruments[i].bytes,
            "pulse instruments");
    }
    const auto wave_instruments = in.word_at(13);
    for (std::size_t i = 0; i < kInstruments; ++i) {
        in.read(
            static_cast<std::uint16_t>(wave_instruments + i * kWaveInstrumentSize),
            song.wave_instruments[i].bytes,
            "wave instruments");
    }
    const auto noise = in.word_at(15);
    for (std::size_t i = 0; i < kInstruments; ++i) {
        in.read(
            static_cast<std::uint16_t>(noise + i * kNoiseSize),
            song.noise_instruments[i].bytes,
            "noise instruments");
    }
    const auto waves = in.word_at(17);
    for (std::size_t i = 0; i < kWaves; ++i) {
        in.read(static_cast<std::uint16_t>(waves + i * kWaveBytes), song.waves[i], "waves");
    }

    for (const auto address : pattern_addresses) {
        const auto start = in.offset(address, kPatternSize, "pattern");
        Pattern pattern;
        for (std::size_t row = 0; row < kRowsPerPattern; ++row) {
            const auto cell = start + row * kCellSize;
            pattern[row] = Cell {
                in.byte_at(cell),
                static_cast<std::uint8_t>(in.byte_at(cell + 1) >> 4),
                static_cast<std::uint8_t>(in.byte_at(cell + 1) & 0x0F),
                in.byte_at(cell + 2)};
        }
        song.patterns.push_back(pattern);
    }

    validate_song(song);
    return song;
}

} // namespace golem
