#include "golem/edit.h"

#include "golem/instrument_fields.h"
#include "golem/song_text.h"

#include <gtest/gtest.h>

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
