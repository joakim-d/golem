#pragma once

#include "golem/song.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// Editing model of the Golem editor: everything the editor does to a song, without UI.
// See docs/editor-steps/00-editing-model.md.
namespace golem::edit {

// The five columns of a cell: note, instrument, effect code, parameter high and low digits.
enum class Column {
    Note,
    Instrument,
    Effect,
    ParamHigh,
    ParamLow,
};

constexpr int kColumnsPerCell = 5;
constexpr int kLowestOctave = 2;
constexpr int kHighestOctave = 7;

struct Cursor {
    std::size_t order = 0;
    std::size_t row = 0;
    std::size_t channel = 0;
    Column column = Column::Note;
};

bool operator==(
    const Cursor& a,
    const Cursor& b);

// Note of a tracker keyboard key (QWERTY): the lower row (z s x d ... /) plays from C of
// `octave`, the upper row (q 2 w 3 ... p) from C of `octave` + 1. std::nullopt for any other
// key, or a note outside C-2..B-7.
std::optional<std::uint8_t> note_for_key(
    char key,
    int octave);

// Value of a hex digit key (0-9, a-f, A-F), else std::nullopt.
std::optional<std::uint8_t> hex_digit(char key);

// A song being edited.
class Document {
public:
    // A new song, ready to play: one order with patterns 0-3, 6 ticks per row, an audible
    // instrument 1 for each channel type, and wave 0 set to a triangle.
    Document();
    explicit Document(
        Song song,
        std::string path = {});

    // Throws SongError if the file cannot be read, parsed or validated.
    static Document open(const std::string& path);

    // Writes the song as text to path() (or `path`, which becomes the document's path) and
    // marks it unmodified. Throws SongError if the file cannot be written.
    void save();
    void save_as(const std::string& path);

    const Song& song() const;
    const std::string& path() const;
    bool modified() const;
    const Cursor& cursor() const;

    // The cell under the cursor, and the pattern index it belongs to.
    const Cell& cell() const;
    std::uint8_t pattern() const;

    // Entry settings.
    int octave() const;
    void set_octave(int octave); // Clamped to kLowestOctave..kHighestOctave.
    int edit_step() const;
    void set_edit_step(int rows); // Clamped to 0..16.
    std::uint8_t instrument() const;
    void set_instrument(std::uint8_t instrument); // Clamped to 1..15.

    // Cursor movement, clamped to the pattern and to the first/last column.
    void move_rows(int delta);
    void move_columns(int delta); // Crosses channels.
    void move_channels(int delta); // Same column in the next/previous channel.
    void set_order(std::size_t order); // Clamped to the orders; keeps row and column.
    void set_cursor(const Cursor& cursor); // Each part clamped, e.g. for a mouse click.

    // Entry at the cursor. Each one is a change (undoable) and moves the cursor down by the
    // edit step. Returns false, changing nothing, when the key means nothing in this column.
    bool enter_key(char key); // Note column: note_for_key; other columns: hex_digit.
    void enter_note(std::uint8_t note); // Note and current instrument.
    void enter_note_off(); // No note, no instrument, effect C00.
    void enter_hex(std::uint8_t digit); // Instrument, effect code or parameter digit.
    void clear(); // Note and instrument, instrument, or effect and parameter.

    // Orders and patterns. Each is a change; false when a limit forbids it.
    bool insert_order(); // A copy of the current order after it; the cursor moves to it.
    bool remove_order(); // The current order; at least one order stays.
    bool set_pattern(std::uint8_t pattern); // Current channel's pattern in the current order.
    bool new_pattern(); // A new empty pattern for the current channel in the current order.

    void set_ticks_per_row(std::uint8_t ticks); // A change; 0 means 256.

    bool can_undo() const;
    bool can_redo() const;
    void undo(); // Restores the song and the cursor of before the last change.
    void redo();

private:
    struct Snapshot {
        Song song;
        Cursor cursor;
        std::uint64_t revision;
    };

    void change(); // Records the current state for undo, before a change.
    Cell& cell_at_cursor();
    void advance();

    Song song_;
    std::string path_;
    Cursor cursor_;
    int octave_ = 4;
    int edit_step_ = 1;
    std::uint8_t instrument_ = 1;
    std::uint64_t revision_ = 0;
    std::uint64_t next_revision_ = 1;
    std::uint64_t saved_revision_ = 0;
    std::vector<Snapshot> undo_;
    std::vector<Snapshot> redo_;
};

} // namespace golem::edit
