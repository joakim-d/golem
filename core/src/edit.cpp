#include "golem/edit.h"

#include "golem/instrument_fields.h"
#include "golem/notes.h"
#include "golem/song_text.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string_view>

namespace golem::edit {

namespace {

    constexpr std::string_view kLowerRow = "zsxdcvgbhnjm,l.;/"; // From C of the octave.
    constexpr std::string_view kUpperRow = "q2w3er5t6y7ui9o0p"; // From C of the octave above.
    constexpr int kMaxEditStep = 16;
    constexpr std::size_t kMaxPatterns = 256;
    constexpr std::uint8_t kNoteOffEffect = 0xC;

    // The song of a new document: audible straight away.
    Song new_song()
    {
        Song song;
        song.ticks_per_row = 6;
        song.orders.push_back({0, 1, 2, 3});
        song.patterns.resize(kChannels);
        song.pulse_instruments[0].bytes = {0x00, 0x80, 0xF0}; // 50% duty, volume F, held
        song.wave_instruments[0].bytes = {0x00, 0x20}; // Full volume, wave 0
        song.noise_instruments[0].bytes = {0x00, 0xF2}; // Volume F, fading like a drum
        song.waves[0] = wave_preset(WavePreset::Triangle);
        return song;
    }

    std::uint8_t instrument_index(std::uint8_t instrument)
    {
        return static_cast<std::uint8_t>(
            std::clamp<std::uint8_t>(instrument, 1, static_cast<std::uint8_t>(kInstruments)) - 1);
    }

    std::uint8_t wave_index(std::uint8_t wave)
    {
        return std::min<std::uint8_t>(wave, static_cast<std::uint8_t>(kWaves - 1));
    }

    int column_index(const Cursor& cursor)
    {
        return static_cast<int>(cursor.channel) * kColumnsPerCell + static_cast<int>(cursor.column);
    }

} // namespace

bool operator==(
    const Cursor& a,
    const Cursor& b)
{
    return a.order == b.order && a.row == b.row && a.channel == b.channel && a.column == b.column;
}

std::optional<std::uint8_t> note_for_key(
    char key,
    int octave)
{
    int semitone;
    if (const auto lower = kLowerRow.find(key); lower != std::string_view::npos) {
        semitone = static_cast<int>(lower);
    } else if (const auto upper = kUpperRow.find(key); upper != std::string_view::npos) {
        semitone = static_cast<int>(upper) + 12;
    } else {
        return std::nullopt;
    }
    const int note = (octave - kLowestOctave) * 12 + semitone + kFirstNote;
    if (note < kFirstNote || note > kLastNote) {
        return std::nullopt;
    }
    return static_cast<std::uint8_t>(note);
}

std::optional<std::uint8_t> hex_digit(char key)
{
    if (key >= '0' && key <= '9') {
        return static_cast<std::uint8_t>(key - '0');
    }
    if (key >= 'a' && key <= 'f') {
        return static_cast<std::uint8_t>(key - 'a' + 10);
    }
    if (key >= 'A' && key <= 'F') {
        return static_cast<std::uint8_t>(key - 'A' + 10);
    }
    return std::nullopt;
}

Song preview_song(
    const Song& song,
    std::size_t channel,
    std::uint8_t note,
    std::uint8_t instrument)
{
    Song preview = song;
    preview.ticks_per_row = 0; // 256 ticks
    preview.patterns.assign(2, Pattern {}); // 0: empty, 1: the note.
    Order order {0, 0, 0, 0};
    order[std::min(channel, kChannels - 1)] = 1;
    preview.orders = {order};
    preview.patterns[1][0] =
        Cell {note, static_cast<std::uint8_t>(instrument_index(instrument) + 1), 0, 0};
    return preview;
}

Document::Document()
    : Document(new_song())
{
}

Document::Document(
    Song song,
    std::string path)
    : song_(std::move(song))
    , path_(std::move(path))
{
    validate_song(song_);
}

Document Document::open(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw SongError("cannot open " + path);
    }
    std::ostringstream text;
    text << file.rdbuf();
    return Document(parse_song_text(text.str()), path);
}

void Document::save()
{
    if (path_.empty()) {
        throw SongError("the song has no file yet");
    }
    std::ofstream file(path_, std::ios::binary);
    file << format_song_text(song_);
    if (!file) {
        throw SongError("cannot write " + path_);
    }
    saved_revision_ = revision_;
}

void Document::save_as(const std::string& path)
{
    path_ = path;
    save();
}

const Song& Document::song() const
{
    return song_;
}

const std::string& Document::path() const
{
    return path_;
}

bool Document::modified() const
{
    return revision_ != saved_revision_;
}

const Cursor& Document::cursor() const
{
    return cursor_;
}

const Cell& Document::cell() const
{
    return song_.patterns[pattern()][cursor_.row];
}

std::uint8_t Document::pattern() const
{
    return song_.orders[cursor_.order][cursor_.channel];
}

int Document::octave() const
{
    return octave_;
}

void Document::set_octave(int octave)
{
    octave_ = std::clamp(octave, kLowestOctave, kHighestOctave);
}

int Document::edit_step() const
{
    return edit_step_;
}

void Document::set_edit_step(int rows)
{
    edit_step_ = std::clamp(rows, 0, kMaxEditStep);
}

std::uint8_t Document::instrument() const
{
    return instrument_;
}

void Document::set_instrument(std::uint8_t instrument)
{
    instrument_ = std::clamp<std::uint8_t>(instrument, 1, static_cast<std::uint8_t>(kInstruments));
}

void Document::move_rows(int delta)
{
    const int row =
        std::clamp(static_cast<int>(cursor_.row) + delta, 0, static_cast<int>(kRowsPerPattern) - 1);
    cursor_.row = static_cast<std::size_t>(row);
}

void Document::move_columns(int delta)
{
    const int last = static_cast<int>(kChannels) * kColumnsPerCell - 1;
    const int index = std::clamp(column_index(cursor_) + delta, 0, last);
    cursor_.channel = static_cast<std::size_t>(index / kColumnsPerCell);
    cursor_.column = static_cast<Column>(index % kColumnsPerCell);
}

void Document::move_channels(int delta)
{
    const int channel =
        std::clamp(static_cast<int>(cursor_.channel) + delta, 0, static_cast<int>(kChannels) - 1);
    cursor_.channel = static_cast<std::size_t>(channel);
}

void Document::set_order(std::size_t order)
{
    cursor_.order = std::min(order, song_.orders.size() - 1);
}

void Document::set_cursor(const Cursor& cursor)
{
    cursor_.order = std::min(cursor.order, song_.orders.size() - 1);
    cursor_.row = std::min(cursor.row, kRowsPerPattern - 1);
    cursor_.channel = std::min(cursor.channel, kChannels - 1);
    cursor_.column = cursor.column;
}

bool Document::enter_key(char key)
{
    if (cursor_.column == Column::Note) {
        const auto note = note_for_key(key, octave_);
        if (!note) {
            return false;
        }
        enter_note(*note);
        return true;
    }
    const auto digit = hex_digit(key);
    if (!digit) {
        return false;
    }
    enter_hex(*digit);
    return true;
}

void Document::enter_note(std::uint8_t note)
{
    change();
    Cell& cell = cell_at_cursor();
    cell.note = note;
    cell.instrument = instrument_;
    advance();
}

void Document::enter_note_off()
{
    change();
    cell_at_cursor() = Cell {0, 0, kNoteOffEffect, 0x00};
    advance();
}

void Document::enter_hex(std::uint8_t digit)
{
    change();
    Cell& cell = cell_at_cursor();
    digit &= 0x0F;
    switch (cursor_.column) {
    case Column::Instrument:
        cell.instrument = digit;
        break;
    case Column::Effect:
        cell.effect = digit;
        break;
    case Column::ParamHigh:
        cell.param = static_cast<std::uint8_t>(digit << 4 | (cell.param & 0x0F));
        break;
    case Column::ParamLow:
        cell.param = static_cast<std::uint8_t>((cell.param & 0xF0) | digit);
        break;
    case Column::Note:
        break;
    }
    advance();
}

void Document::clear()
{
    change();
    Cell& cell = cell_at_cursor();
    switch (cursor_.column) {
    case Column::Note:
        cell.note = kNoteNone;
        cell.instrument = 0;
        break;
    case Column::Instrument:
        cell.instrument = 0;
        break;
    case Column::Effect:
    case Column::ParamHigh:
    case Column::ParamLow:
        cell.effect = 0;
        cell.param = 0;
        break;
    }
    advance();
}

bool Document::insert_order()
{
    if (song_.orders.size() >= kMaxOrders) {
        return false;
    }
    change();
    const Order copy = song_.orders[cursor_.order];
    song_.orders.insert(
        song_.orders.begin() + static_cast<std::ptrdiff_t>(cursor_.order) + 1, copy);
    ++cursor_.order;
    return true;
}

bool Document::remove_order()
{
    if (song_.orders.size() <= 1) {
        return false;
    }
    change();
    song_.orders.erase(song_.orders.begin() + static_cast<std::ptrdiff_t>(cursor_.order));
    cursor_.order = std::min(cursor_.order, song_.orders.size() - 1);
    return true;
}

bool Document::set_pattern(std::uint8_t pattern)
{
    change();
    if (song_.patterns.size() <= pattern) {
        song_.patterns.resize(std::size_t {pattern} + 1);
    }
    song_.orders[cursor_.order][cursor_.channel] = pattern;
    return true;
}

bool Document::new_pattern()
{
    if (song_.patterns.size() >= kMaxPatterns) {
        return false;
    }
    change();
    song_.orders[cursor_.order][cursor_.channel] = static_cast<std::uint8_t>(song_.patterns.size());
    song_.patterns.emplace_back();
    return true;
}

void Document::set_ticks_per_row(std::uint8_t ticks)
{
    change();
    song_.ticks_per_row = ticks;
}

void Document::set_pulse_instrument(
    std::uint8_t instrument,
    const PulseInstrument& value)
{
    const std::uint8_t index = instrument_index(instrument);
    if (song_.pulse_instruments[index].bytes == value.bytes) {
        return;
    }
    merged_change(Target::Pulse, index);
    song_.pulse_instruments[index] = value;
}

void Document::set_wave_instrument(
    std::uint8_t instrument,
    const WaveInstrument& value)
{
    const std::uint8_t index = instrument_index(instrument);
    if (song_.wave_instruments[index].bytes == value.bytes) {
        return;
    }
    merged_change(Target::WaveInstrument, index);
    song_.wave_instruments[index] = value;
}

void Document::set_noise_instrument(
    std::uint8_t instrument,
    const NoiseInstrument& value)
{
    const std::uint8_t index = instrument_index(instrument);
    if (song_.noise_instruments[index].bytes == value.bytes) {
        return;
    }
    merged_change(Target::Noise, index);
    song_.noise_instruments[index] = value;
}

void Document::set_wave(
    std::uint8_t wave,
    const Wave& value)
{
    const std::uint8_t index = wave_index(wave);
    if (song_.waves[index] == value) {
        return;
    }
    merged_change(Target::Wave, index);
    song_.waves[index] = value;
}

void Document::set_wave_sample(
    std::uint8_t wave,
    std::size_t index,
    std::uint8_t sample)
{
    index = std::min(index, kWaveSamples - 1);
    Wave value = song_.waves[wave_index(wave)];
    std::uint8_t& byte = value[index / 2];
    sample &= 0x0F;
    byte = static_cast<std::uint8_t>(
        index % 2 == 0 ? (sample << 4 | (byte & 0x0F)) : ((byte & 0xF0) | sample));
    set_wave(wave, value);
}

void Document::finish_edit()
{
    merging_.reset();
}

bool Document::can_undo() const
{
    return !undo_.empty();
}

bool Document::can_redo() const
{
    return !redo_.empty();
}

void Document::undo()
{
    if (undo_.empty()) {
        return;
    }
    redo_.push_back({song_, cursor_, revision_});
    Snapshot& previous = undo_.back();
    song_ = std::move(previous.song);
    cursor_ = previous.cursor;
    revision_ = previous.revision;
    undo_.pop_back();
    merging_.reset();
}

void Document::redo()
{
    if (redo_.empty()) {
        return;
    }
    undo_.push_back({song_, cursor_, revision_});
    Snapshot& next = redo_.back();
    song_ = std::move(next.song);
    cursor_ = next.cursor;
    revision_ = next.revision;
    redo_.pop_back();
    merging_.reset();
}

void Document::change()
{
    undo_.push_back({song_, cursor_, revision_});
    redo_.clear();
    revision_ = next_revision_++;
    merging_.reset();
}

void Document::merged_change(
    Target target,
    std::uint8_t index)
{
    const auto key = std::make_pair(target, index);
    if (merging_ != key) {
        change();
        merging_ = key;
    } else {
        revision_ = next_revision_++; // Still a change for the modified flag.
    }
}

Cell& Document::cell_at_cursor()
{
    return song_.patterns[pattern()][cursor_.row];
}

void Document::advance()
{
    move_rows(edit_step_);
}

} // namespace golem::edit
