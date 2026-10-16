#include "golem/edit.h"

#include "golem/instrument_fields.h"
#include "golem/notes.h"
#include "golem/song_text.h"

#include <algorithm>
#include <fstream>
#include <set>
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

    constexpr std::size_t kFields = kChannels * kFieldsPerCell;

    // Row and field (counted across channels) of the cursor, for the selection's ends.
    std::pair<
        std::size_t,
        std::size_t>
    position(const Cursor& cursor)
    {
        return {
            cursor.row,
            cursor.channel * kFieldsPerCell + static_cast<std::size_t>(field_of(cursor.column))};
    }

    void copy_field(
        Cell& to,
        const Cell& from,
        Field field)
    {
        switch (field) {
        case Field::Note:
            to.note = from.note;
            break;
        case Field::Instrument:
            to.instrument = from.instrument;
            break;
        case Field::Effect:
            to.effect = from.effect;
            to.param = from.param;
            break;
        }
    }

    void clear_field(
        Cell& cell,
        Field field)
    {
        copy_field(cell, Cell {}, field);
    }

} // namespace

bool operator==(
    const Cursor& a,
    const Cursor& b)
{
    return a.order == b.order && a.row == b.row && a.channel == b.channel && a.column == b.column;
}

Field field_of(Column column)
{
    switch (column) {
    case Column::Note:
        return Field::Note;
    case Column::Instrument:
        return Field::Instrument;
    case Column::Effect:
    case Column::ParamHigh:
    case Column::ParamLow:
        break;
    }
    return Field::Effect;
}

bool Selection::contains(
    std::size_t row,
    std::size_t channel,
    Column column) const
{
    const std::size_t field = channel * kFieldsPerCell + static_cast<std::size_t>(field_of(column));
    return row >= first_row && row <= last_row && field >= first_field && field <= last_field;
}

bool operator==(
    const Selection& a,
    const Selection& b)
{
    return a.first_row == b.first_row
        && a.last_row == b.last_row
        && a.first_field == b.first_field
        && a.last_field == b.last_field;
}

bool Clip::empty() const
{
    return fields == 0 || rows.empty();
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

void Document::move_rows(
    int delta,
    bool select)
{
    Cursor cursor = cursor_;
    const int row =
        std::clamp(static_cast<int>(cursor.row) + delta, 0, static_cast<int>(kRowsPerPattern) - 1);
    cursor.row = static_cast<std::size_t>(row);
    move_to(cursor, select);
}

void Document::move_columns(
    int delta,
    bool select)
{
    Cursor cursor = cursor_;
    const int last = static_cast<int>(kChannels) * kColumnsPerCell - 1;
    const int index = std::clamp(column_index(cursor) + delta, 0, last);
    cursor.channel = static_cast<std::size_t>(index / kColumnsPerCell);
    cursor.column = static_cast<Column>(index % kColumnsPerCell);
    move_to(cursor, select);
}

void Document::move_channels(
    int delta,
    bool select)
{
    Cursor cursor = cursor_;
    const int channel =
        std::clamp(static_cast<int>(cursor.channel) + delta, 0, static_cast<int>(kChannels) - 1);
    cursor.channel = static_cast<std::size_t>(channel);
    move_to(cursor, select);
}

void Document::set_order(std::size_t order)
{
    order = std::min(order, song_.orders.size() - 1);
    if (order != cursor_.order) {
        anchor_.reset();
    }
    cursor_.order = order;
}

void Document::set_cursor(
    const Cursor& cursor,
    bool select)
{
    Cursor clamped;
    clamped.order = std::min(cursor.order, song_.orders.size() - 1);
    clamped.row = std::min(cursor.row, kRowsPerPattern - 1);
    clamped.channel = std::min(cursor.channel, kChannels - 1);
    clamped.column = cursor.column;
    move_to(clamped, select);
}

std::optional<Selection> Document::selection() const
{
    if (!anchor_) {
        return std::nullopt;
    }
    return Selection {
        std::min(anchor_->first, end_.first),
        std::max(anchor_->first, end_.first),
        std::min(anchor_->second, end_.second),
        std::max(anchor_->second, end_.second)};
}

void Document::select_all()
{
    anchor_ = std::make_pair(std::size_t {0}, std::size_t {0});
    end_ = {kRowsPerPattern - 1, kFields - 1};
}

void Document::clear_selection()
{
    anchor_.reset();
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
    if (const auto selected = selection()) {
        change();
        clear_block(*selected);
        return;
    }
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

Clip Document::copy() const
{
    const Selection block = this->block();
    const Order& order = song_.orders[cursor_.order];
    const std::size_t first_channel = block.first_field / kFieldsPerCell;
    Clip clip;
    clip.first_field = static_cast<Field>(block.first_field % kFieldsPerCell);
    clip.fields = block.last_field - block.first_field + 1;
    for (std::size_t row = block.first_row; row <= block.last_row; ++row) {
        std::vector<Cell> cells(block.last_field / kFieldsPerCell - first_channel + 1);
        for (std::size_t field = block.first_field; field <= block.last_field; ++field) {
            const std::size_t channel = field / kFieldsPerCell;
            copy_field(
                cells[channel - first_channel],
                song_.patterns[order[channel]][row],
                static_cast<Field>(field % kFieldsPerCell));
        }
        clip.rows.push_back(std::move(cells));
    }
    return clip;
}

Clip Document::cut()
{
    Clip clip = copy();
    change();
    clear_block(block());
    return clip;
}

bool Document::paste(const Clip& clip)
{
    if (clip.empty()) {
        return false;
    }
    change();
    const Order& order = song_.orders[cursor_.order];
    const std::size_t first = static_cast<std::size_t>(clip.first_field);
    const std::size_t base = cursor_.channel * kFieldsPerCell + first;
    std::size_t last_row = cursor_.row;
    for (std::size_t i = 0; i < clip.rows.size() && cursor_.row + i < kRowsPerPattern; ++i) {
        last_row = cursor_.row + i;
        for (std::size_t f = 0; f < clip.fields && base + f < kFields; ++f) {
            const std::size_t from_channel = (first + f) / kFieldsPerCell;
            if (from_channel >= clip.rows[i].size()) {
                break;
            }
            copy_field(
                song_.patterns[order[(base + f) / kFieldsPerCell]][last_row],
                clip.rows[i][from_channel],
                static_cast<Field>((first + f) % kFieldsPerCell));
        }
    }
    anchor_ = std::make_pair(cursor_.row, base);
    end_ = {last_row, std::min(base + clip.fields, kFields) - 1};
    return true;
}

bool Document::transpose(int semitones)
{
    // Each pattern cell once, even when channels share a pattern.
    const Selection block = this->block();
    const Order& order = song_.orders[cursor_.order];
    std::set<std::pair<std::uint8_t, std::size_t>> notes; // Pattern and row.
    for (std::size_t channel = 0; channel < kChannels; ++channel) {
        const std::size_t field = channel * kFieldsPerCell + static_cast<std::size_t>(Field::Note);
        if (field < block.first_field || field > block.last_field) {
            continue;
        }
        for (std::size_t row = block.first_row; row <= block.last_row; ++row) {
            const std::uint8_t note = song_.patterns[order[channel]][row].note;
            if (note == kNoteNone) {
                continue;
            }
            const int moved = note + semitones;
            if (moved < kFirstNote || moved > kLastNote) {
                return false;
            }
            notes.emplace(order[channel], row);
        }
    }
    if (notes.empty()) {
        return false;
    }
    change();
    for (const auto& [pattern, row] : notes) {
        std::uint8_t& note = song_.patterns[pattern][row].note;
        note = static_cast<std::uint8_t>(note + semitones);
    }
    return true;
}

bool Document::insert_order()
{
    if (song_.orders.size() >= kMaxOrders) {
        return false;
    }
    change();
    anchor_.reset();
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
    anchor_.reset();
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
    anchor_.reset();
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
    anchor_.reset();
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

void Document::move_to(
    const Cursor& cursor,
    bool select)
{
    const bool same_order = cursor.order == cursor_.order;
    if (select && same_order) {
        if (!anchor_) {
            anchor_ = position(cursor_);
        }
        end_ = position(cursor);
    } else {
        anchor_.reset();
    }
    cursor_ = cursor;
}

void Document::clear_block(const Selection& block)
{
    const Order& order = song_.orders[cursor_.order];
    for (std::size_t row = block.first_row; row <= block.last_row; ++row) {
        for (std::size_t field = block.first_field; field <= block.last_field; ++field) {
            clear_field(
                song_.patterns[order[field / kFieldsPerCell]][row],
                static_cast<Field>(field % kFieldsPerCell));
        }
    }
}

Selection Document::block() const
{
    if (const auto selected = selection()) {
        return *selected;
    }
    const std::size_t first = cursor_.channel * kFieldsPerCell;
    return Selection {cursor_.row, cursor_.row, first, first + kFieldsPerCell - 1};
}

} // namespace golem::edit
