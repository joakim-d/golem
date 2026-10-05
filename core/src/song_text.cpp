#include "golem/song_text.h"

#include "golem/notes.h"

#include <array>
#include <cstdio>
#include <set>
#include <vector>

namespace golem {

namespace {

    constexpr std::array<const char*, 12> kNoteNames = {
        "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};
    constexpr int kFirstOctave = 2;
    constexpr std::uint8_t kDefaultTicksPerRow = 6;

    std::optional<unsigned> parse_hex(
        std::string_view token,
        std::size_t max_digits)
    {
        if (token.empty() || token.size() > max_digits) {
            return std::nullopt;
        }
        unsigned value = 0;
        for (char c : token) {
            unsigned digit;
            if (c >= '0' && c <= '9') {
                digit = c - '0';
            } else if (c >= 'A' && c <= 'F') {
                digit = c - 'A' + 10;
            } else if (c >= 'a' && c <= 'f') {
                digit = c - 'a' + 10;
            } else {
                return std::nullopt;
            }
            value = value << 4 | digit;
        }
        return value;
    }

    std::string hex(
        unsigned value,
        int digits)
    {
        char buffer[9];
        std::snprintf(buffer, sizeof buffer, "%0*X", digits, value);
        return buffer;
    }

    std::vector<std::string_view> tokenize(std::string_view line)
    {
        // A '#' starts a comment only at the start of a word, so that "C#4" stays a note.
        for (std::size_t i = 0; i < line.size(); ++i) {
            if (line[i] == '#' && (i == 0 || line[i - 1] == ' ' || line[i - 1] == '\t')) {
                line = line.substr(0, i);
                break;
            }
        }
        std::vector<std::string_view> tokens;
        std::size_t pos = 0;
        while (pos < line.size()) {
            const auto start = line.find_first_not_of(" \t\r", pos);
            if (start == std::string_view::npos) {
                break;
            }
            const auto end = std::min(line.find_first_of(" \t\r", start), line.size());
            tokens.push_back(line.substr(start, end - start));
            pos = end;
        }
        return tokens;
    }

    class Parser {
    public:
        Song parse(std::string_view text)
        {
            std::size_t pos = 0;
            while (pos <= text.size()) {
                const auto end = std::min(text.find('\n', pos), text.size());
                ++line_;
                parse_line(tokenize(text.substr(pos, end - pos)));
                pos = end + 1;
            }
            validate_song(song_);
            return song_;
        }

    private:
        [[noreturn]] void fail(const std::string& message) const
        {
            throw SongError("line " + std::to_string(line_) + ": " + message);
        }

        unsigned hex_arg(
            std::string_view token,
            std::size_t max_digits,
            unsigned min,
            unsigned max,
            const char* what) const
        {
            const auto value = parse_hex(token, max_digits);
            if (!value || *value < min || *value > max) {
                fail(std::string("invalid ") + what + " '" + std::string(token) + "'");
            }
            return *value;
        }

        void expect_args(
            const std::vector<std::string_view>& tokens,
            std::size_t count) const
        {
            if (tokens.size() != count + 1) {
                fail(std::string(tokens[0]) + " expects " + std::to_string(count) + " argument(s)");
            }
        }

        template <std::size_t N>
        void parse_instrument(
            const std::vector<std::string_view>& tokens,
            std::array<
                std::uint8_t,
                N>& bytes)
        {
            for (std::size_t i = 0; i < N; ++i) {
                bytes[i] = static_cast<std::uint8_t>(hex_arg(tokens[2 + i], 2, 0, 0xFF, "byte"));
            }
        }

        template <typename Instruments>
        void parse_instruments(
            const std::vector<std::string_view>& tokens,
            Instruments& instruments,
            std::set<unsigned>& defined)
        {
            constexpr std::size_t size = std::tuple_size<decltype(instruments[0].bytes)>::value;
            expect_args(tokens, 1 + size);
            const auto number = hex_arg(tokens[1], 1, 1, kInstruments, "instrument number");
            if (!defined.insert(number).second) {
                fail("instrument " + hex(number, 1) + " defined twice");
            }
            parse_instrument(tokens, instruments[number - 1].bytes);
        }

        void parse_line(const std::vector<std::string_view>& tokens)
        {
            if (tokens.empty()) {
                return;
            }
            const auto directive = tokens[0];
            if (directive == "ticks_per_row") {
                expect_args(tokens, 1);
                song_.ticks_per_row =
                    static_cast<std::uint8_t>(hex_arg(tokens[1], 2, 0, 0xFF, "tempo"));
            } else if (directive == "pulse") {
                parse_instruments(tokens, song_.pulse_instruments, pulse_defined_);
            } else if (directive == "wave_instrument") {
                parse_instruments(tokens, song_.wave_instruments, wave_instrument_defined_);
            } else if (directive == "noise") {
                parse_instruments(tokens, song_.noise_instruments, noise_defined_);
            } else if (directive == "wave") {
                parse_wave(tokens);
            } else if (directive == "order") {
                expect_args(tokens, kChannels);
                Order order;
                for (std::size_t channel = 0; channel < kChannels; ++channel) {
                    order[channel] = static_cast<std::uint8_t>(
                        hex_arg(tokens[1 + channel], 2, 0, 0xFF, "pattern index"));
                }
                song_.orders.push_back(order);
            } else if (directive == "pattern") {
                expect_args(tokens, 1);
                const auto index = hex_arg(tokens[1], 2, 0, 0xFF, "pattern index");
                if (!patterns_defined_.insert(index).second) {
                    fail("pattern " + hex(index, 2) + " defined twice");
                }
                if (song_.patterns.size() <= index) {
                    song_.patterns.resize(index + 1);
                }
                pattern_ = index;
                rows_defined_.clear();
            } else if (parse_hex(directive, 2)) {
                parse_row(tokens);
            } else {
                fail("unknown directive '" + std::string(directive) + "'");
            }
        }

        void parse_wave(const std::vector<std::string_view>& tokens)
        {
            expect_args(tokens, 2);
            const auto index = hex_arg(tokens[1], 1, 0, kWaves - 1, "wave index");
            if (!wave_defined_.insert(index).second) {
                fail("wave " + hex(index, 1) + " defined twice");
            }
            const auto samples = tokens[2];
            if (samples.size() != 2 * kWaveBytes) {
                fail("wave expects " + std::to_string(2 * kWaveBytes) + " hex digits");
            }
            for (std::size_t i = 0; i < kWaveBytes; ++i) {
                song_.waves[index][i] = static_cast<std::uint8_t>(
                    hex_arg(samples.substr(2 * i, 2), 2, 0, 0xFF, "wave byte"));
            }
        }

        void parse_row(const std::vector<std::string_view>& tokens)
        {
            if (!pattern_) {
                fail("row outside of a pattern");
            }
            if (tokens.size() != 4) {
                fail("row expects: row note instrument effect");
            }
            const auto row = hex_arg(tokens[0], 2, 0, kRowsPerPattern - 1, "row");
            if (!rows_defined_.insert(row).second) {
                fail("row " + hex(row, 2) + " defined twice");
            }
            Cell cell;
            const auto note = parse_note_name(tokens[1]);
            if (!note) {
                fail("invalid note '" + std::string(tokens[1]) + "'");
            }
            cell.note = *note;
            if (tokens[2] != ".") {
                cell.instrument =
                    static_cast<std::uint8_t>(hex_arg(tokens[2], 1, 1, kInstruments, "instrument"));
            }
            if (tokens[3] != "...") {
                if (tokens[3].size() != 3) {
                    fail("invalid effect '" + std::string(tokens[3]) + "'");
                }
                const auto effect = hex_arg(tokens[3], 3, 0, 0xFFF, "effect");
                cell.effect = static_cast<std::uint8_t>(effect >> 8);
                cell.param = static_cast<std::uint8_t>(effect & 0xFF);
            }
            song_.patterns[*pattern_][row] = cell;
        }

        Song song_ = [] {
            Song song;
            song.ticks_per_row = kDefaultTicksPerRow;
            return song;
        }();
        int line_ = 0;
        std::optional<unsigned> pattern_;
        std::set<unsigned> patterns_defined_, rows_defined_, wave_defined_;
        std::set<unsigned> pulse_defined_, wave_instrument_defined_, noise_defined_;
    };

    template <std::size_t N>
    std::string hex_bytes(
        const std::array<
            std::uint8_t,
            N>& bytes)
    {
        std::string text;
        for (auto byte : bytes) {
            text += ' ' + hex(byte, 2);
        }
        return text;
    }

    template <typename Instruments>
    void format_instruments(
        std::string& text,
        const char* directive,
        const Instruments& instruments)
    {
        for (std::size_t i = 0; i < instruments.size(); ++i) {
            const auto& bytes = instruments[i].bytes;
            if (bytes != std::remove_reference_t<decltype(bytes)> {}) {
                text += std::string(directive)
                      + ' '
                      + hex(static_cast<unsigned>(i + 1), 1)
                      + hex_bytes(bytes)
                      + '\n';
            }
        }
    }

} // namespace

Song parse_song_text(std::string_view text)
{
    return Parser().parse(text);
}

std::string format_song_text(const Song& song)
{
    std::string text = "ticks_per_row " + hex(song.ticks_per_row, 2) + "\n";

    std::string instruments;
    format_instruments(instruments, "pulse", song.pulse_instruments);
    format_instruments(instruments, "wave_instrument", song.wave_instruments);
    format_instruments(instruments, "noise", song.noise_instruments);
    for (std::size_t i = 0; i < kWaves; ++i) {
        if (song.waves[i] != Wave {}) {
            instruments += "wave " + hex(static_cast<unsigned>(i), 1) + ' ';
            for (auto byte : song.waves[i]) {
                instruments += hex(byte, 2);
            }
            instruments += '\n';
        }
    }
    if (!instruments.empty()) {
        text += '\n' + instruments;
    }

    text += '\n';
    for (const auto& order : song.orders) {
        text += "order" + hex_bytes(order) + '\n';
    }

    for (std::size_t p = 0; p < song.patterns.size(); ++p) {
        text += "\npattern " + hex(static_cast<unsigned>(p), 2) + '\n';
        for (std::size_t row = 0; row < kRowsPerPattern; ++row) {
            const Cell& cell = song.patterns[p][row];
            if (cell.empty()) {
                continue;
            }
            text += hex(static_cast<unsigned>(row), 2)
                  + ' '
                  + note_name(cell.note)
                  + ' '
                  + (cell.instrument ? hex(cell.instrument, 1) : ".")
                  + ' '
                  + (cell.effect || cell.param ? hex(cell.effect, 1) + hex(cell.param, 2) : "...")
                  + '\n';
        }
    }
    return text;
}

std::string note_name(std::uint8_t note)
{
    if (note == kNoteNone) {
        return "---";
    }
    if (note > kLastNote) {
        throw std::out_of_range("note out of range: " + std::to_string(note));
    }
    const int index = note - kFirstNote;
    return kNoteNames[index % 12] + std::to_string(kFirstOctave + index / 12);
}

std::optional<std::uint8_t> parse_note_name(std::string_view name)
{
    if (name == "---") {
        return kNoteNone;
    }
    if (name.size() != 3 || name[2] < '0' + kFirstOctave || name[2] > '9') {
        return std::nullopt;
    }
    for (std::size_t semitone = 0; semitone < kNoteNames.size(); ++semitone) {
        if (name.substr(0, 2) == kNoteNames[semitone]) {
            const int note =
                kFirstNote + (name[2] - '0' - kFirstOctave) * 12 + static_cast<int>(semitone);
            if (note > kLastNote) {
                return std::nullopt;
            }
            return static_cast<std::uint8_t>(note);
        }
    }
    return std::nullopt;
}

} // namespace golem
