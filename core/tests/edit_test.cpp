#include "golem/edit.h"

#include "golem/instrument_fields.h"
#include "golem/notes.h"
#include "golem/player.h"
#include "golem/song_text.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>

using namespace golem;
using namespace golem::edit;

namespace {

constexpr std::uint8_t kC4 = 25;

// A temporary file path, removed at the end of the test.
class TempFile {
public:
    explicit TempFile(const std::string& name)
        : path_(std::filesystem::temp_directory_path() / ("golem-edit-test-" + name))
    {
    }

    ~TempFile()
    {
        std::error_code ignored;
        std::filesystem::remove(path_, ignored);
    }

    std::string str() const
    {
        return path_.string();
    }

private:
    std::filesystem::path path_;
};

Cursor at(
    std::size_t order,
    std::size_t row,
    std::size_t channel,
    Column column)
{
    return Cursor {order, row, channel, column};
}

} // namespace

// --- Keys ---

TEST(
    Edit,
    KeyboardMap)
{
    EXPECT_EQ(note_for_key('z', 4), kC4); // C-4
    EXPECT_EQ(note_for_key('s', 4), kC4 + 1); // C#4
    EXPECT_EQ(note_for_key('m', 4), kC4 + 11); // B-4
    EXPECT_EQ(note_for_key(',', 4), kC4 + 12); // C-5 on the lower row
    EXPECT_EQ(note_for_key('/', 4), kC4 + 16); // E-5
    EXPECT_EQ(note_for_key('q', 4), kC4 + 12); // C-5 on the upper row
    EXPECT_EQ(note_for_key('2', 4), kC4 + 13);
    EXPECT_EQ(note_for_key('p', 4), kC4 + 28); // E-6
    EXPECT_EQ(note_for_key('z', 2), 1); // C-2
    EXPECT_EQ(note_for_key('m', 7), 72); // B-7
}

TEST(
    Edit,
    KeyboardMapRefusesOtherKeysAndNotesOutOfRange)
{
    EXPECT_EQ(note_for_key('a', 4), std::nullopt);
    EXPECT_EQ(note_for_key('1', 4), std::nullopt);
    EXPECT_EQ(note_for_key('Z', 4), std::nullopt);
    EXPECT_EQ(note_for_key('q', 7), std::nullopt); // C-8
    EXPECT_EQ(note_for_key(',', 7), std::nullopt);
}

TEST(
    Edit,
    HexDigits)
{
    EXPECT_EQ(hex_digit('0'), 0);
    EXPECT_EQ(hex_digit('9'), 9);
    EXPECT_EQ(hex_digit('a'), 0xA);
    EXPECT_EQ(hex_digit('F'), 0xF);
    EXPECT_EQ(hex_digit('g'), std::nullopt);
    EXPECT_EQ(hex_digit('.'), std::nullopt);
}

// --- New document ---

TEST(
    Edit,
    ANewDocumentIsReadyToPlay)
{
    const Document doc;
    const Song& song = doc.song();
    EXPECT_NO_THROW(validate_song(song));
    ASSERT_EQ(song.orders.size(), 1u);
    EXPECT_EQ(song.orders[0], (Order {0, 1, 2, 3}));
    EXPECT_EQ(song.patterns.size(), 4u);
    EXPECT_EQ(song.ticks_per_row, 6);
    EXPECT_NE(song.pulse_instruments[0].nrx2() >> 4, 0); // Audible.
    EXPECT_NE(song.wave_instruments[0].nr32(), 0);
    EXPECT_NE(song.noise_instruments[0].nr42() >> 4, 0);
    EXPECT_NE(song.waves[0], Wave {});
    EXPECT_FALSE(doc.modified());
    EXPECT_TRUE(doc.path().empty());
    EXPECT_EQ(doc.cursor(), Cursor {});
    EXPECT_FALSE(doc.can_undo());
    EXPECT_FALSE(doc.can_redo());
}

TEST(
    Edit,
    SettingsAreClamped)
{
    Document doc;
    doc.set_octave(1);
    EXPECT_EQ(doc.octave(), kLowestOctave);
    doc.set_octave(9);
    EXPECT_EQ(doc.octave(), kHighestOctave);
    doc.set_edit_step(-1);
    EXPECT_EQ(doc.edit_step(), 0);
    doc.set_edit_step(20);
    EXPECT_EQ(doc.edit_step(), 16);
    doc.set_instrument(0);
    EXPECT_EQ(doc.instrument(), 1);
    doc.set_instrument(20);
    EXPECT_EQ(doc.instrument(), 15);
    EXPECT_FALSE(doc.modified()); // Settings are not changes to the song.
}

// --- Entry ---

TEST(
    Edit,
    ANoteGetsTheCurrentInstrumentAndTheCursorMovesDown)
{
    Document doc;
    doc.set_instrument(3);
    doc.enter_note(kC4);
    EXPECT_EQ(doc.song().patterns[0][0], (Cell {kC4, 3, 0, 0}));
    EXPECT_EQ(doc.cursor(), at(0, 1, 0, Column::Note));
    EXPECT_TRUE(doc.modified());
}

TEST(
    Edit,
    EditStep)
{
    Document doc;
    doc.set_edit_step(0);
    doc.enter_note(kC4);
    EXPECT_EQ(doc.cursor().row, 0u);
    doc.set_edit_step(4);
    doc.enter_note(kC4);
    EXPECT_EQ(doc.cursor().row, 4u);
}

TEST(
    Edit,
    TheCursorStopsOnTheLastRow)
{
    Document doc;
    doc.move_rows(63);
    doc.set_edit_step(4);
    doc.enter_note(kC4);
    EXPECT_EQ(doc.cursor().row, 63u);
    EXPECT_EQ(doc.song().patterns[0][63].note, kC4);
}

TEST(
    Edit,
    NoteKeysUseTheOctave)
{
    Document doc;
    doc.set_octave(5);
    EXPECT_TRUE(doc.enter_key('z'));
    EXPECT_EQ(doc.song().patterns[0][0].note, kC4 + 12);
}

TEST(
    Edit,
    KeysThatMeanNothingChangeNothing)
{
    Document doc;
    EXPECT_FALSE(doc.enter_key('a')); // Not a note key.
    doc.set_octave(7);
    EXPECT_FALSE(doc.enter_key('q')); // C-8
    doc.move_columns(1); // Instrument column
    EXPECT_FALSE(doc.enter_key('z')); // Not a hex digit.
    EXPECT_FALSE(doc.modified());
    EXPECT_FALSE(doc.can_undo());
    EXPECT_EQ(doc.cursor(), at(0, 0, 0, Column::Instrument));
}

TEST(
    Edit,
    NoteOff)
{
    Document doc;
    doc.enter_note(kC4);
    doc.move_rows(-1);
    doc.enter_note_off();
    EXPECT_EQ(doc.song().patterns[0][0], (Cell {0, 0, 0xC, 0x00}));
}

TEST(
    Edit,
    HexEntryInEachColumn)
{
    Document doc;
    doc.set_edit_step(0);
    doc.move_columns(1);
    doc.enter_hex(0xA); // Instrument
    doc.move_columns(1);
    EXPECT_TRUE(doc.enter_key('4')); // Effect code
    doc.move_columns(1);
    doc.enter_hex(0x1); // Parameter, high digit
    doc.move_columns(1);
    EXPECT_TRUE(doc.enter_key('F')); // Parameter, low digit
    EXPECT_EQ(doc.cell(), (Cell {0, 0xA, 0x4, 0x1F}));
    doc.move_columns(-3);
    doc.enter_hex(0); // Instrument 0: none.
    EXPECT_EQ(doc.cell().instrument, 0);
}

TEST(
    Edit,
    ClearEmptiesTheFieldUnderTheCursor)
{
    Document doc;
    doc.set_edit_step(0);
    doc.enter_note(kC4);
    doc.move_columns(2);
    doc.enter_hex(0xC);
    doc.move_columns(1);
    doc.enter_hex(0x8); // C80
    doc.move_columns(-3);
    doc.clear(); // Note column: note and instrument.
    EXPECT_EQ(doc.cell(), (Cell {0, 0, 0xC, 0x80}));
    doc.move_columns(3);
    doc.clear(); // Parameter column: effect and parameter.
    EXPECT_EQ(doc.cell(), Cell {});
}

// --- Cursor ---

TEST(
    Edit,
    CursorMovementIsClamped)
{
    Document doc;
    doc.move_rows(-5);
    EXPECT_EQ(doc.cursor().row, 0u);
    doc.move_rows(100);
    EXPECT_EQ(doc.cursor().row, 63u);
    doc.move_columns(-1);
    EXPECT_EQ(doc.cursor(), at(0, 63, 0, Column::Note));
    doc.move_columns(4 * kColumnsPerCell + 10);
    EXPECT_EQ(doc.cursor(), at(0, 63, 3, Column::ParamLow));
    doc.set_order(9);
    EXPECT_EQ(doc.cursor().order, 0u);
}

TEST(
    Edit,
    ColumnsCrossChannels)
{
    Document doc;
    doc.move_columns(kColumnsPerCell - 1);
    EXPECT_EQ(doc.cursor(), at(0, 0, 0, Column::ParamLow));
    doc.move_columns(1);
    EXPECT_EQ(doc.cursor(), at(0, 0, 1, Column::Note));
    doc.move_columns(-1);
    EXPECT_EQ(doc.cursor(), at(0, 0, 0, Column::ParamLow));
    doc.move_channels(2);
    EXPECT_EQ(doc.cursor(), at(0, 0, 2, Column::ParamLow));
    doc.move_channels(5);
    EXPECT_EQ(doc.cursor().channel, 3u);
}

TEST(
    Edit,
    SetCursorIsClamped)
{
    Document doc;
    doc.insert_order();
    doc.set_cursor(at(1, 10, 2, Column::Effect));
    EXPECT_EQ(doc.cursor(), at(1, 10, 2, Column::Effect));
    doc.set_cursor(at(5, 99, 9, Column::ParamLow));
    EXPECT_EQ(doc.cursor(), at(1, 63, 3, Column::ParamLow));
    doc.undo(); // Undoes insert_order: moving the cursor added no undo step.
    EXPECT_EQ(doc.song().orders.size(), 1u);
    EXPECT_FALSE(doc.can_undo());
}

// --- Orders and patterns ---

TEST(
    Edit,
    InsertOrderCopiesTheCurrentOneAndSharesItsPatterns)
{
    Document doc;
    EXPECT_TRUE(doc.insert_order());
    EXPECT_EQ(doc.song().orders.size(), 2u);
    EXPECT_EQ(doc.song().orders[1], (Order {0, 1, 2, 3}));
    EXPECT_EQ(doc.cursor().order, 1u);
    doc.enter_note(kC4); // Pattern 0, also used by order 0.
    doc.set_order(0);
    EXPECT_EQ(doc.song().patterns[0][0].note, kC4);
    doc.move_rows(-1);
    EXPECT_EQ(doc.cell().note, kC4);
}

TEST(
    Edit,
    OrdersAreLimitedTo255)
{
    Document doc;
    for (int i = 1; i < 255; ++i) {
        ASSERT_TRUE(doc.insert_order());
    }
    EXPECT_FALSE(doc.insert_order());
    EXPECT_EQ(doc.song().orders.size(), 255u);
}

TEST(
    Edit,
    RemoveOrderKeepsAtLeastOne)
{
    Document doc;
    EXPECT_FALSE(doc.remove_order());
    doc.insert_order();
    doc.set_pattern(7);
    EXPECT_TRUE(doc.remove_order()); // Order 1, the cursor's.
    EXPECT_EQ(doc.song().orders.size(), 1u);
    EXPECT_EQ(doc.cursor().order, 0u);
}

TEST(
    Edit,
    SetPatternCreatesEmptyPatternsUpToIt)
{
    Document doc;
    doc.move_channels(2);
    EXPECT_TRUE(doc.set_pattern(6));
    EXPECT_EQ(doc.song().orders[0], (Order {0, 1, 6, 3}));
    EXPECT_EQ(doc.song().patterns.size(), 7u);
    EXPECT_EQ(doc.pattern(), 6);
    EXPECT_NO_THROW(validate_song(doc.song()));
}

TEST(
    Edit,
    NewPatternIsEmptyAndUnused)
{
    Document doc;
    doc.enter_note(kC4); // Pattern 0
    EXPECT_TRUE(doc.new_pattern());
    EXPECT_EQ(doc.song().orders[0][0], 4);
    EXPECT_EQ(doc.song().patterns[4], Pattern {});
    EXPECT_EQ(doc.song().patterns[0][0].note, kC4);
}

TEST(
    Edit,
    PatternsAreLimitedTo256)
{
    Document doc;
    EXPECT_TRUE(doc.set_pattern(255));
    EXPECT_EQ(doc.song().patterns.size(), 256u);
    EXPECT_FALSE(doc.new_pattern());
}

TEST(
    Edit,
    TicksPerRowIsAChange)
{
    Document doc;
    doc.set_ticks_per_row(3);
    EXPECT_EQ(doc.song().ticks_per_row, 3);
    EXPECT_TRUE(doc.modified());
    doc.undo();
    EXPECT_EQ(doc.song().ticks_per_row, 6);
}

// --- Undo and redo ---

TEST(
    Edit,
    UndoRestoresTheSongAndTheCursor)
{
    Document doc;
    doc.enter_note(kC4);
    doc.enter_note(kC4 + 2);
    ASSERT_TRUE(doc.can_undo());
    doc.undo();
    EXPECT_EQ(doc.song().patterns[0][1], Cell {});
    EXPECT_EQ(doc.cursor(), at(0, 1, 0, Column::Note));
    doc.undo();
    EXPECT_EQ(doc.song().patterns[0][0], Cell {});
    EXPECT_EQ(doc.cursor(), Cursor {});
    EXPECT_FALSE(doc.can_undo());
    doc.redo();
    doc.redo();
    EXPECT_EQ(doc.song().patterns[0][1].note, kC4 + 2);
    EXPECT_EQ(doc.cursor(), at(0, 2, 0, Column::Note));
    EXPECT_FALSE(doc.can_redo());
}

TEST(
    Edit,
    ANewChangeClearsRedo)
{
    Document doc;
    doc.enter_note(kC4);
    doc.undo();
    ASSERT_TRUE(doc.can_redo());
    doc.enter_note(kC4 + 1);
    EXPECT_FALSE(doc.can_redo());
}

TEST(
    Edit,
    ModifiedFollowsTheSavedState)
{
    TempFile file("modified.gsong");
    Document doc;
    doc.enter_note(kC4);
    EXPECT_TRUE(doc.modified());
    doc.undo();
    EXPECT_FALSE(doc.modified());
    doc.redo();
    EXPECT_TRUE(doc.modified());
    doc.save_as(file.str());
    EXPECT_FALSE(doc.modified());
    EXPECT_EQ(doc.path(), file.str());
    doc.undo();
    EXPECT_TRUE(doc.modified()); // Different from what was saved.
    doc.redo();
    EXPECT_FALSE(doc.modified());
}

// --- Instruments and waves ---

TEST(
    Edit,
    SetInstrumentsIsAChange)
{
    Document doc;
    const Song original = doc.song();
    doc.set_pulse_instrument(2, PulseInstrument {{0x00, 0x40, 0xA1}});
    doc.set_wave_instrument(3, WaveInstrument {{0x10, 0x41}});
    doc.set_noise_instrument(15, NoiseInstrument {{0x80, 0x71}});
    EXPECT_EQ(
        doc.song().pulse_instruments[1].bytes, (std::array<std::uint8_t, 3> {0x00, 0x40, 0xA1}));
    EXPECT_EQ(doc.song().wave_instruments[2].bytes, (std::array<std::uint8_t, 2> {0x10, 0x41}));
    EXPECT_EQ(doc.song().noise_instruments[14].bytes, (std::array<std::uint8_t, 2> {0x80, 0x71}));
    EXPECT_TRUE(doc.modified());
    doc.undo();
    doc.undo();
    doc.undo();
    EXPECT_EQ(doc.song(), original);
    EXPECT_FALSE(doc.modified());
    EXPECT_FALSE(doc.can_undo());
}

TEST(
    Edit,
    SetWavesIsAChange)
{
    Document doc;
    const Wave saw = wave_preset(WavePreset::Saw);
    doc.set_wave(4, saw);
    EXPECT_EQ(doc.song().waves[4], saw);
    doc.set_wave_sample(5, 0, 0xF); // High nibble of byte 0.
    doc.set_wave_sample(5, 1, 0x3); // Low nibble.
    doc.set_wave_sample(5, 31, 0x1C); // Masked to C.
    EXPECT_EQ(doc.song().waves[5][0], 0xF3);
    EXPECT_EQ(doc.song().waves[5][15], 0x0C);
    EXPECT_TRUE(doc.modified());
}

TEST(
    Edit,
    InstrumentAndWaveNumbersAreClamped)
{
    Document doc;
    doc.set_pulse_instrument(0, PulseInstrument {{0x00, 0x00, 0x11}}); // Instrument 1
    doc.set_pulse_instrument(20, PulseInstrument {{0x00, 0x00, 0x22}}); // Instrument 15
    doc.set_wave(16, wave_preset(WavePreset::Square)); // Wave 15
    doc.set_wave_sample(0, 40, 0x5); // Sample 31
    EXPECT_EQ(doc.song().pulse_instruments[0].bytes[2], 0x11);
    EXPECT_EQ(doc.song().pulse_instruments[14].bytes[2], 0x22);
    EXPECT_EQ(doc.song().waves[15], wave_preset(WavePreset::Square));
    EXPECT_EQ(wave_sample(doc.song().waves[0], 31), 0x5);
}

TEST(
    Edit,
    SettingTheCurrentValueChangesNothing)
{
    Document doc;
    doc.set_pulse_instrument(1, doc.song().pulse_instruments[0]);
    doc.set_wave_instrument(1, doc.song().wave_instruments[0]);
    doc.set_noise_instrument(1, doc.song().noise_instruments[0]);
    doc.set_wave(0, doc.song().waves[0]);
    doc.set_wave_sample(0, 3, wave_sample(doc.song().waves[0], 3));
    EXPECT_FALSE(doc.modified());
    EXPECT_FALSE(doc.can_undo());
}

TEST(
    Edit,
    ChangesToTheSameInstrumentAreOneUndoStep)
{
    Document doc;
    const Song original = doc.song();
    for (std::uint8_t volume = 1; volume <= 8; ++volume) { // A slider being dragged.
        doc.set_pulse_instrument(
            1, PulseInstrument {{0x00, 0x80, static_cast<std::uint8_t>(volume << 4)}});
    }
    EXPECT_EQ(doc.song().pulse_instruments[0].bytes[2], 0x80);
    doc.undo();
    EXPECT_EQ(doc.song(), original);
    EXPECT_FALSE(doc.can_undo());
    doc.redo();
    EXPECT_EQ(doc.song().pulse_instruments[0].bytes[2], 0x80);
}

TEST(
    Edit,
    ChangesToTheSameWaveAreOneUndoStep)
{
    Document doc;
    const Song original = doc.song();
    for (std::size_t index = 0; index < kWaveSamples; ++index) { // A wave being drawn.
        doc.set_wave_sample(1, index, 0xF);
    }
    doc.set_wave(1, wave_preset(WavePreset::Sine));
    doc.undo();
    EXPECT_EQ(doc.song(), original);
    EXPECT_FALSE(doc.can_undo());
}

TEST(
    Edit,
    FinishEditEndsAnUndoStep)
{
    Document doc;
    doc.set_wave_sample(0, 0, 0x1);
    doc.finish_edit();
    doc.set_wave_sample(0, 0, 0x2);
    doc.undo();
    EXPECT_EQ(wave_sample(doc.song().waves[0], 0), 0x1);
    doc.undo();
    EXPECT_EQ(wave_sample(doc.song().waves[0], 0), 0x0);
    EXPECT_FALSE(doc.can_undo());
}

TEST(
    Edit,
    AnotherTargetOrChangeEndsAnUndoStep)
{
    Document doc;
    doc.set_pulse_instrument(1, PulseInstrument {{0x00, 0x00, 0x10}});
    doc.set_pulse_instrument(2, PulseInstrument {{0x00, 0x00, 0x10}}); // Another instrument
    doc.set_wave_instrument(2, WaveInstrument {{0x00, 0x40}}); // Same number, another type
    doc.set_wave(2, wave_preset(WavePreset::Saw));
    doc.enter_note(kC4); // Another kind of change
    doc.set_wave(2, wave_preset(WavePreset::Square));
    int steps = 0;
    while (doc.can_undo()) {
        doc.undo();
        ++steps;
    }
    EXPECT_EQ(steps, 6);
}

TEST(
    Edit,
    UndoAndRedoEndAnUndoStep)
{
    Document doc;
    doc.set_wave_sample(0, 0, 0x1);
    doc.set_wave_sample(0, 0, 0x2);
    doc.undo();
    doc.redo();
    doc.set_wave_sample(0, 0, 0x3); // A new step: undo goes back to 2, not to 0.
    doc.undo();
    EXPECT_EQ(wave_sample(doc.song().waves[0], 0), 0x2);
}

TEST(
    Edit,
    MergedChangesAfterASaveAreModified)
{
    TempFile file("merged.gsong");
    Document doc;
    doc.set_wave_sample(0, 0, 0x1);
    doc.save_as(file.str());
    EXPECT_FALSE(doc.modified());
    doc.set_wave_sample(0, 0, 0x2); // Same undo step as before the save.
    EXPECT_TRUE(doc.modified());
    doc.undo();
    EXPECT_TRUE(doc.modified()); // Back to the state before the step, not the saved one.
}

// --- Selection ---

namespace {

// Field index of a channel's field, counted across channels.
std::size_t field(
    std::size_t channel,
    Field kind)
{
    return channel * kFieldsPerCell + static_cast<std::size_t>(kind);
}

// A cell that says where it is: note row + 1, instrument channel + 1, effect A, parameter row.
Cell numbered(
    std::size_t channel,
    std::size_t row)
{
    return Cell {
        static_cast<std::uint8_t>(row + 1),
        static_cast<std::uint8_t>(channel + 1),
        0xA,
        static_cast<std::uint8_t>(row)};
}

// A new document's song (patterns 0-3 on channels 1-4) with every cell numbered.
Document numbered_document()
{
    Song song = Document().song();
    for (std::size_t channel = 0; channel < kChannels; ++channel) {
        for (std::size_t row = 0; row < kRowsPerPattern; ++row) {
            song.patterns[song.orders[0][channel]][row] = numbered(channel, row);
        }
    }
    return Document(song);
}

const Cell& cell_at(
    const Document& doc,
    std::size_t channel,
    std::size_t row)
{
    const Song& song = doc.song();
    return song.patterns[song.orders[doc.cursor().order][channel]][row];
}

} // namespace

TEST(
    Edit,
    FieldsOfColumns)
{
    EXPECT_EQ(field_of(Column::Note), Field::Note);
    EXPECT_EQ(field_of(Column::Instrument), Field::Instrument);
    EXPECT_EQ(field_of(Column::Effect), Field::Effect);
    EXPECT_EQ(field_of(Column::ParamHigh), Field::Effect);
    EXPECT_EQ(field_of(Column::ParamLow), Field::Effect);
}

TEST(
    Edit,
    SelectionContainsItsFields)
{
    const Selection selection {2, 5, field(1, Field::Instrument), field(2, Field::Note)};
    EXPECT_TRUE(selection.contains(2, 1, Column::Instrument));
    EXPECT_TRUE(selection.contains(5, 1, Column::ParamLow));
    EXPECT_TRUE(selection.contains(3, 2, Column::Note));
    EXPECT_FALSE(selection.contains(3, 1, Column::Note));
    EXPECT_FALSE(selection.contains(3, 2, Column::Instrument));
    EXPECT_FALSE(selection.contains(1, 1, Column::Instrument));
    EXPECT_FALSE(selection.contains(6, 1, Column::Instrument));
}

TEST(
    Edit,
    NothingIsSelectedAtFirst)
{
    const Document doc;
    EXPECT_EQ(doc.selection(), std::nullopt);
}

TEST(
    Edit,
    SelectingExtendsFromWhereTheCursorWas)
{
    Document doc;
    doc.set_cursor(at(0, 2, 1, Column::Instrument));
    doc.move_rows(3, true);
    EXPECT_EQ(
        doc.selection(),
        (Selection {2, 5, field(1, Field::Instrument), field(1, Field::Instrument)}));
    doc.move_columns(2, true); // Parameter: the effect field.
    EXPECT_EQ(
        doc.selection(), (Selection {2, 5, field(1, Field::Instrument), field(1, Field::Effect)}));
    doc.move_columns(2, true); // Across to channel 3's note.
    EXPECT_EQ(
        doc.selection(), (Selection {2, 5, field(1, Field::Instrument), field(2, Field::Note)}));
    EXPECT_EQ(doc.cursor(), at(0, 5, 2, Column::Note));
}

TEST(
    Edit,
    SelectingUpAndLeft)
{
    Document doc;
    doc.set_cursor(at(0, 10, 2, Column::Effect));
    doc.move_rows(-4, true);
    doc.move_channels(-2, true);
    EXPECT_EQ(
        doc.selection(), (Selection {6, 10, field(0, Field::Effect), field(2, Field::Effect)}));
    doc.move_rows(8, true); // Back past the anchor.
    EXPECT_EQ(
        doc.selection(), (Selection {10, 14, field(0, Field::Effect), field(2, Field::Effect)}));
}

TEST(
    Edit,
    SelectingWithTheMouse)
{
    Document doc;
    doc.set_cursor(at(0, 1, 0, Column::Note));
    doc.set_cursor(at(0, 4, 3, Column::ParamHigh), true);
    EXPECT_EQ(doc.selection(), (Selection {1, 4, 0, field(3, Field::Effect)}));
    doc.set_cursor(at(0, 0, 1, Column::Note), true); // The anchor stays.
    EXPECT_EQ(doc.selection(), (Selection {0, 1, 0, field(1, Field::Note)}));
}

TEST(
    Edit,
    MovingWithoutSelectingClearsTheSelection)
{
    Document doc;
    doc.move_rows(2, true);
    doc.move_rows(1);
    EXPECT_EQ(doc.selection(), std::nullopt);
    doc.move_rows(2, true);
    doc.set_cursor(at(0, 9, 0, Column::Note));
    EXPECT_EQ(doc.selection(), std::nullopt);
    doc.move_rows(2, true);
    doc.clear_selection();
    EXPECT_EQ(doc.selection(), std::nullopt);
    doc.move_rows(2, true);
    doc.enter_note(kC4); // Entries move the cursor too.
    EXPECT_EQ(doc.selection(), std::nullopt);
}

TEST(
    Edit,
    SelectAllKeepsTheCursor)
{
    Document doc;
    doc.set_cursor(at(0, 7, 2, Column::Instrument));
    doc.select_all();
    EXPECT_EQ(
        doc.selection(), (Selection {0, kRowsPerPattern - 1, 0, kChannels * kFieldsPerCell - 1}));
    EXPECT_EQ(doc.cursor(), at(0, 7, 2, Column::Instrument));
}

TEST(
    Edit,
    AnotherOrderClearsTheSelection)
{
    Document doc;
    doc.insert_order();
    doc.select_all();
    doc.set_order(1); // The same order.
    EXPECT_NE(doc.selection(), std::nullopt);
    doc.set_order(0);
    EXPECT_EQ(doc.selection(), std::nullopt);
    doc.select_all();
    doc.insert_order();
    EXPECT_EQ(doc.selection(), std::nullopt);
}

TEST(
    Edit,
    UndoAndRedoClearTheSelection)
{
    Document doc;
    doc.enter_note(kC4);
    doc.select_all();
    doc.undo();
    EXPECT_EQ(doc.selection(), std::nullopt);
    doc.select_all();
    doc.redo();
    EXPECT_EQ(doc.selection(), std::nullopt);
}

// --- Copy, cut, clear ---

TEST(
    Edit,
    CopyTakesTheSelectedFields)
{
    Document doc = numbered_document();
    doc.set_cursor(at(0, 1, 0, Column::Instrument));
    doc.set_cursor(at(0, 2, 1, Column::Note), true);
    const Clip clip = doc.copy();
    EXPECT_EQ(clip.first_field, Field::Instrument);
    EXPECT_EQ(clip.fields, 3u);
    ASSERT_EQ(clip.rows.size(), 2u);
    for (std::size_t row = 0; row < 2; ++row) {
        ASSERT_EQ(clip.rows[row].size(), 2u);
        const Cell first = numbered(0, row + 1);
        const Cell second = numbered(1, row + 1);
        EXPECT_EQ(clip.rows[row][0], (Cell {0, first.instrument, first.effect, first.param}));
        EXPECT_EQ(clip.rows[row][1], (Cell {second.note, 0, 0, 0}));
    }
    EXPECT_FALSE(clip.empty());
    EXPECT_FALSE(doc.can_undo()); // Not a change.
    EXPECT_FALSE(doc.modified());
}

TEST(
    Edit,
    CopyWithoutASelectionTakesTheCursorCell)
{
    Document doc = numbered_document();
    doc.set_cursor(at(0, 5, 2, Column::ParamLow));
    const Clip clip = doc.copy();
    EXPECT_EQ(clip.first_field, Field::Note);
    EXPECT_EQ(clip.fields, kFieldsPerCell);
    ASSERT_EQ(clip.rows.size(), 1u);
    ASSERT_EQ(clip.rows[0].size(), 1u);
    EXPECT_EQ(clip.rows[0][0], numbered(2, 5));
}

TEST(
    Edit,
    CutClearsWhatItCopies)
{
    Document doc = numbered_document();
    const Song before = doc.song();
    doc.set_cursor(at(0, 3, 1, Column::Effect));
    doc.set_cursor(at(0, 4, 2, Column::Instrument), true);
    const Clip clip = doc.cut();
    EXPECT_EQ(clip.rows.size(), 2u);
    EXPECT_EQ(clip.first_field, Field::Effect);
    EXPECT_EQ(clip.fields, 3u);
    for (std::size_t row = 3; row <= 4; ++row) {
        EXPECT_EQ(cell_at(doc, 1, row), (Cell {numbered(1, row).note, 2, 0, 0}));
        EXPECT_EQ(cell_at(doc, 2, row), (Cell {0, 0, 0xA, static_cast<std::uint8_t>(row)}));
    }
    EXPECT_EQ(cell_at(doc, 1, 5), numbered(1, 5));
    EXPECT_EQ(doc.cursor(), at(0, 4, 2, Column::Instrument));
    doc.undo();
    EXPECT_EQ(doc.song(), before);
    EXPECT_FALSE(doc.can_undo()); // One step.
    doc.redo();
    EXPECT_EQ(cell_at(doc, 1, 3).effect, 0);
}

TEST(
    Edit,
    CutWithoutASelectionTakesTheCursorCell)
{
    Document doc = numbered_document();
    doc.set_cursor(at(0, 5, 2, Column::Note));
    const Clip clip = doc.cut();
    EXPECT_EQ(clip.rows[0][0], numbered(2, 5));
    EXPECT_TRUE(cell_at(doc, 2, 5).empty());
    EXPECT_EQ(doc.cursor(), at(0, 5, 2, Column::Note)); // Unlike clear(), it stays.
}

TEST(
    Edit,
    ClearEmptiesTheSelection)
{
    Document doc = numbered_document();
    doc.set_cursor(at(0, 0, 3, Column::Effect));
    doc.move_rows(63, true);
    doc.clear();
    for (std::size_t row = 0; row < kRowsPerPattern; ++row) {
        EXPECT_EQ(cell_at(doc, 3, row), (Cell {numbered(3, row).note, 4, 0, 0}));
    }
    EXPECT_EQ(cell_at(doc, 2, 0), numbered(2, 0));
    EXPECT_EQ(doc.cursor(), at(0, 63, 3, Column::Effect));
    EXPECT_NE(doc.selection(), std::nullopt);
    doc.undo();
    EXPECT_EQ(cell_at(doc, 3, 0), numbered(3, 0));
    EXPECT_FALSE(doc.can_undo());
}

// --- Paste ---

TEST(
    Edit,
    PasteKeepsFieldKinds)
{
    Document source = numbered_document();
    source.set_cursor(at(0, 0, 0, Column::Effect));
    source.move_rows(1, true);
    const Clip effects = source.copy();

    Document doc; // Another song.
    doc.set_cursor(at(0, 10, 2, Column::Note)); // An effect still goes to an effect field.
    EXPECT_TRUE(doc.paste(effects));
    EXPECT_EQ(cell_at(doc, 2, 10), (Cell {0, 0, 0xA, 0}));
    EXPECT_EQ(cell_at(doc, 2, 11), (Cell {0, 0, 0xA, 1}));
    EXPECT_TRUE(cell_at(doc, 2, 12).empty());
    EXPECT_TRUE(cell_at(doc, 3, 10).empty());
    EXPECT_EQ(
        doc.selection(), (Selection {10, 11, field(2, Field::Effect), field(2, Field::Effect)}));
    EXPECT_EQ(doc.cursor(), at(0, 10, 2, Column::Note));
}

TEST(
    Edit,
    PasteOnlyWritesTheClipFields)
{
    Document source = numbered_document();
    source.set_cursor(at(0, 0, 0, Column::Instrument));
    source.set_cursor(at(0, 0, 1, Column::Note), true);
    const Clip clip = source.copy(); // Channel 1's instrument and effect, channel 2's note.

    Document doc = numbered_document();
    doc.set_cursor(at(0, 9, 1, Column::Effect));
    EXPECT_TRUE(doc.paste(clip));
    const Cell from = numbered(0, 0);
    EXPECT_EQ(cell_at(doc, 1, 9), (Cell {numbered(1, 9).note, from.instrument, 0xA, 0}));
    EXPECT_EQ(cell_at(doc, 2, 9), (Cell {1, 3, 0xA, 9}));
}

TEST(
    Edit,
    PasteDropsWhatFallsOutside)
{
    Document source = numbered_document();
    source.select_all();
    const Clip all = source.copy();
    EXPECT_EQ(all.rows.size(), kRowsPerPattern);
    EXPECT_EQ(all.fields, kChannels * kFieldsPerCell);

    Document doc;
    doc.set_cursor(at(0, 60, 2, Column::Instrument));
    EXPECT_TRUE(doc.paste(all));
    for (std::size_t row = 0; row < 4; ++row) {
        EXPECT_EQ(cell_at(doc, 2, 60 + row), numbered(0, row));
        EXPECT_EQ(cell_at(doc, 3, 60 + row), numbered(1, row));
    }
    EXPECT_TRUE(cell_at(doc, 2, 59).empty());
    EXPECT_TRUE(cell_at(doc, 1, 60).empty());
    EXPECT_EQ(
        doc.selection(),
        (Selection {
            60, kRowsPerPattern - 1, field(2, Field::Note), kChannels * kFieldsPerCell - 1}));
}

TEST(
    Edit,
    PasteIsOneUndoStep)
{
    Document source = numbered_document();
    source.select_all();
    const Clip all = source.copy();
    Document doc;
    const Song before = doc.song();
    doc.paste(all);
    EXPECT_TRUE(doc.modified());
    doc.undo();
    EXPECT_EQ(doc.song(), before);
    EXPECT_FALSE(doc.can_undo());
    doc.redo();
    EXPECT_EQ(doc.song().patterns, source.song().patterns);
}

TEST(
    Edit,
    PastingAnEmptyClipChangesNothing)
{
    Document doc;
    EXPECT_TRUE(Clip {}.empty());
    EXPECT_FALSE(doc.paste(Clip {}));
    EXPECT_FALSE(doc.can_undo());
    EXPECT_EQ(doc.selection(), std::nullopt);
}

// --- Transpose ---

TEST(
    Edit,
    TransposeMovesTheSelectedNotes)
{
    Document doc = numbered_document();
    doc.set_cursor(at(0, 0, 0, Column::Note));
    doc.set_cursor(at(0, 3, 1, Column::Note), true);
    EXPECT_TRUE(doc.transpose(12));
    for (std::size_t row = 0; row <= 3; ++row) {
        for (std::size_t channel = 0; channel <= 1; ++channel) {
            Cell expected = numbered(channel, row);
            expected.note = static_cast<std::uint8_t>(expected.note + 12);
            EXPECT_EQ(cell_at(doc, channel, row), expected);
        }
    }
    EXPECT_EQ(cell_at(doc, 0, 4), numbered(0, 4));
    EXPECT_EQ(cell_at(doc, 2, 0), numbered(2, 0));
    EXPECT_TRUE(doc.transpose(-11));
    EXPECT_EQ(cell_at(doc, 1, 3).note, numbered(1, 3).note + 1);
    doc.undo();
    doc.undo();
    EXPECT_EQ(doc.song(), numbered_document().song());
    EXPECT_FALSE(doc.can_undo());
}

TEST(
    Edit,
    TransposeOnlyMovesNoteFields)
{
    Document doc = numbered_document();
    doc.set_cursor(at(0, 0, 0, Column::Instrument));
    doc.set_cursor(at(0, 3, 0, Column::Effect), true);
    EXPECT_FALSE(doc.transpose(1));
    EXPECT_FALSE(doc.can_undo());
}

TEST(
    Edit,
    TransposeLeavesEmptyNotes)
{
    Document doc;
    doc.set_cursor(at(0, 2, 0, Column::Note));
    doc.enter_note(kC4);
    doc.set_cursor(at(0, 0, 0, Column::Note));
    doc.move_rows(5, true);
    EXPECT_TRUE(doc.transpose(2));
    EXPECT_EQ(cell_at(doc, 0, 2).note, kC4 + 2);
    EXPECT_EQ(cell_at(doc, 0, 1).note, kNoteNone);
    EXPECT_EQ(cell_at(doc, 0, 3).note, kNoteNone);

    doc.set_cursor(at(0, 10, 0, Column::Note));
    doc.move_rows(5, true);
    EXPECT_FALSE(doc.transpose(2)); // No note to move.
}

TEST(
    Edit,
    TransposeRefusesNotesOutOfRange)
{
    Document doc;
    doc.enter_note(kLastNote);
    doc.enter_note(kFirstNote);
    const Song before = doc.song();
    doc.set_cursor(at(0, 0, 0, Column::Note));
    doc.move_rows(1, true);
    EXPECT_FALSE(doc.transpose(1)); // B-7 cannot go up...
    EXPECT_FALSE(doc.transpose(-1)); // ...nor C-2 down.
    EXPECT_EQ(doc.song(), before);
    doc.set_cursor(at(0, 0, 0, Column::Note));
    EXPECT_TRUE(doc.transpose(-1)); // Without a selection: the cursor's note alone.
    EXPECT_EQ(cell_at(doc, 0, 0).note, kLastNote - 1);
    EXPECT_EQ(cell_at(doc, 0, 1).note, kFirstNote);
}

TEST(
    Edit,
    TransposeMovesASharedPatternOnce)
{
    Song song = Document().song();
    song.orders = {{0, 0, 0, 0}};
    song.patterns[0][0] = Cell {kC4, 1, 0, 0};
    Document doc(song);
    doc.select_all();
    EXPECT_TRUE(doc.transpose(1));
    EXPECT_EQ(doc.song().patterns[0][0].note, kC4 + 1);
}

// --- Note preview ---

TEST(
    Edit,
    PreviewSongPlaysOneNote)
{
    Document doc;
    doc.set_wave(3, wave_preset(WavePreset::Sine));
    doc.enter_note(kC4 + 7); // The preview ignores the song's own patterns and orders.
    const Song preview = preview_song(doc.song(), 2, kC4, 3);
    Song same_sounds = doc.song(); // The song with the preview's orders, patterns and speed.
    same_sounds.orders = preview.orders;
    same_sounds.patterns = preview.patterns;
    same_sounds.ticks_per_row = preview.ticks_per_row;
    EXPECT_EQ(preview, same_sounds); // Same instruments and waves.
    EXPECT_EQ(preview.ticks_per_row, 0); // 256 ticks: the note is never retriggered.
    ASSERT_EQ(preview.orders.size(), 1u);
    const Order& order = preview.orders[0];
    for (std::size_t channel = 0; channel < kChannels; ++channel) {
        const Pattern& pattern = preview.patterns[order[channel]];
        for (std::size_t row = 0; row < kRowsPerPattern; ++row) {
            const Cell expected = channel == 2 && row == 0 ? Cell {kC4, 3, 0, 0} : Cell {};
            EXPECT_EQ(pattern[row], expected) << "channel " << channel << " row " << row;
        }
    }
    EXPECT_NO_THROW(validate_song(preview));
}

TEST(
    Edit,
    PreviewSongTriggersTheChannel)
{
    const Document doc;
    const std::uint16_t triggers[] = {reg::NR14, reg::NR24, reg::NR34, reg::NR44};
    for (std::size_t channel = 0; channel < kChannels; ++channel) {
        const auto frames = render(preview_song(doc.song(), channel, kC4, 1), 2);
        const auto& writes = frames[1]; // The first row.
        const bool triggered = std::any_of(writes.begin(), writes.end(), [&](const ApuWrite& w) {
            return w.address == triggers[channel] && (w.value & 0x80) != 0;
        });
        EXPECT_TRUE(triggered) << "channel " << channel;
        for (const auto& write : writes) {
            for (std::size_t other = 0; other < kChannels; ++other) {
                if (other != channel) {
                    EXPECT_NE(write.address, triggers[other]) << "channel " << channel;
                }
            }
        }
    }
}

TEST(
    Edit,
    PreviewSongClampsTheInstrument)
{
    const Document doc;
    for (const auto& [instrument, expected] : {std::pair<int, int> {0, 1}, {16, 15}}) {
        const Song preview =
            preview_song(doc.song(), 0, kC4, static_cast<std::uint8_t>(instrument));
        EXPECT_EQ(preview.patterns[preview.orders[0][0]][0].instrument, expected);
    }
}

// --- Files ---

TEST(
    Edit,
    SaveAndOpenRoundTrip)
{
    TempFile file("round-trip.gsong");
    Document doc;
    doc.enter_note(kC4);
    doc.insert_order();
    doc.save_as(file.str());
    const Document opened = Document::open(file.str());
    EXPECT_EQ(opened.song(), doc.song());
    EXPECT_EQ(opened.path(), file.str());
    EXPECT_FALSE(opened.modified());
    EXPECT_FALSE(opened.can_undo());
}

TEST(
    Edit,
    SaveWithoutAPathFails)
{
    Document doc;
    EXPECT_THROW(doc.save(), SongError);
}

TEST(
    Edit,
    OpenFailsOnMissingOrInvalidFiles)
{
    EXPECT_THROW(Document::open("/nonexistent/golem/song.gsong"), SongError);
    TempFile file("invalid.gsong");
    std::ofstream(file.str()) << "order 00 00 00 05\npattern 00\n"; // No pattern 05.
    EXPECT_THROW(Document::open(file.str()), SongError);
}
