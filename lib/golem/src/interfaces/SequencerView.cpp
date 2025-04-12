#include <interfaces/SequencerView.h>

#include <golem/domain/Track.h>
#include <golem/use_cases/ProjectUseCases.h>
#include <golem/utils/scope_guard.h>

#include <interfaces/GuiState.h>
#include <interfaces/ImGuiTools.h>

namespace
{

static ImColor COLORS[12] {
    ImColor { 0x22, 0x22, 0x22 }, ImColor { 0x11, 0x11, 0x11 },
    ImColor { 0x22, 0x22, 0x22 }, ImColor { 0x11, 0x11, 0x11 },
    ImColor { 0x22, 0x22, 0x22 }, ImColor { 0x11, 0x11, 0x11 },
    ImColor { 0x22, 0x22, 0x22 }, ImColor { 0x22, 0x22, 0x22 },
    ImColor { 0x11, 0x11, 0x11 }, ImColor { 0x22, 0x22, 0x22 },
    ImColor { 0x11, 0x11, 0x11 }, ImColor { 0x22, 0x22, 0x22 },
};

constexpr int CELL_WIDTH = 15;
constexpr int CELL_HEIGHT = 13;
constexpr int CELL_SPACING = 1;
constexpr int NOTES_BY_PHRASE = 16;
constexpr int PHRASE_COUNT = 16;
constexpr int PHRASE_WIDTH = ((CELL_WIDTH + CELL_SPACING) * NOTES_BY_PHRASE);

}

namespace gui
{

void drawProgressionBar(
    use_cases::ProjectUseCases& use_cases,
    size_t song_index,
    float x_offset)
{
    const auto& playback_info = use_cases.get_playback_info.execute();

    const unsigned int ticks_per_note
        = use_cases.get_ticks_per_note.execute(song_index);

    if (ticks_per_note == 0)
    {
        return;
    }

    WindowDrawHelper window_draw_helper;

    const auto window_size = ImGui::GetWindowSize();

    const int x_position
        = ((float)(playback_info.current_tick) / float(ticks_per_note))
            * CELL_WIDTH
        + playback_info.current_note * (CELL_WIDTH + CELL_SPACING) - x_offset;

    window_draw_helper.drawFilledRect(
        { float(x_position), 0.f }, { 1, window_size.y },
        ImColor { 0xFF, 0, 0 });
}

SequencerView::SequencerView(
    use_cases::ProjectUseCases& use_cases,
    ImVec2 offset)
{
    ImGui::BeginChild(
        "SequencerView", ImVec2 { -1, -1 }, 0, ImGuiWindowFlags_NoScrollbar);

    utils::ScopeGuard scope_guard { ImGui::EndChild };

    WindowDrawHelper window_draw_helper;

    ImGui::Dummy(ImVec2 { -1, -1 });

    const auto window_size = ImGui::GetWindowSize();

    static bool note_edition = false;
    static int note_pressed_index;
    static int note_moving_index;
    static domain::NoteFrequency note_pressed_frequency;

    const int cell_by_row = (window_size.x / (CELL_WIDTH + CELL_SPACING)) + 1;
    const int cell_by_column
        = (window_size.y / (CELL_HEIGHT + CELL_SPACING)) + 1;
    const int start_j = offset.y / (CELL_HEIGHT + CELL_SPACING);
    const int start_i = offset.x / (CELL_WIDTH + CELL_SPACING);

    const int draw_cells_on_row
        = start_i + cell_by_row > NOTES_BY_PHRASE * PHRASE_COUNT
        ? (16 * 16) - start_i
        : cell_by_row;

    const int draw_cells_on_column
        = start_j + cell_by_column > 12 * 6 // 12 notes * 6 octaves
        ? (12 * 6) - start_j
        : cell_by_column;

    const int start_x_offset = (static_cast<int>(offset.x) % (CELL_WIDTH + 1));
    const int start_y_offset = (static_cast<int>(offset.y) % (CELL_HEIGHT + 1));

    auto song_index = GuiState::songIndex();
    auto track_index = GuiState::trackIndex();

    ImVec2 size = ImVec2 { CELL_WIDTH, CELL_HEIGHT };
    ImVec2 position;
    int rect_index = 0;
    for (int j = -1; j <= draw_cells_on_column; j++)
    {
        for (int i = -1; i <= draw_cells_on_row; i++)
        {
            position.x = i * (CELL_WIDTH + CELL_SPACING) - start_x_offset;
            position.y = j * (CELL_HEIGHT + CELL_SPACING) - start_y_offset;

            window_draw_helper.drawFilledRect(
                position, size, COLORS[(j + start_j) % 12]);
        }
    }

    auto note_color
        = note_edition ? ImColor { 255, 0, 0, 128 } : ImColor { 0xFF, 0, 0 };

    if (IsMouseChildClicked(ImGuiMouseButton_Left))
    {
        const auto position = GetMouseToWindowPosition();

        note_pressed_index
            = (position.x + offset.x) / (CELL_WIDTH + CELL_SPACING);

        note_moving_index = note_pressed_index;

        note_pressed_frequency = static_cast<domain::NoteFrequency>(
            (6 * 12) - (position.y + offset.y) / (CELL_HEIGHT + CELL_SPACING));

        use_cases.play_note.execute(
            GuiState::instrumentIndex(),
            static_cast<domain::Channel>(track_index), note_pressed_frequency);
        note_edition = true;
    }

    if (IsMouseChildDown(ImGuiMouseButton_Left))
    {
        const auto position = GetMouseToWindowPosition();
        note_moving_index
            = (position.x + offset.x) / (CELL_WIDTH + CELL_SPACING);
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && note_edition)
    {
        const auto position = GetMouseToWindowPosition();
        note_edition = false;
        const int note_released_index
            = (position.x + offset.x) / (CELL_WIDTH + CELL_SPACING);

        const unsigned note_duration
            = std::abs(note_pressed_index - note_released_index) + 1;

        const int absolute_note_index
            = std::min(note_pressed_index, note_released_index);

        use_cases.stop_note.execute(static_cast<domain::Channel>(track_index));

        use_cases.add_note.execute(
            song_index, track_index, absolute_note_index,
            domain::Note { note_pressed_frequency, note_duration,
                           GuiState::instrumentIndex() });
    }

    drawProgressionBar(use_cases, song_index, offset.x);

    // Draw notes
    const size_t notes_per_phrase = use_cases.get_notes_per_phrase(song_index);

    const auto track = use_cases.get_track(song_index, track_index);

    if (!track)
    {
        return;
    }

    for (int i = 0; i < draw_cells_on_row; i++)
    {
        const auto note = track->note(i + start_i);

        if (!note.has_value())
            continue;

        position.x = i * (CELL_WIDTH + CELL_SPACING) - start_x_offset + 1;
        position.y = ((6 * 12 - int(note->frequency())) - 1)
                * (CELL_HEIGHT + CELL_SPACING)
            - offset.y + 1;
        size.x = (CELL_WIDTH + CELL_SPACING) * note->duration() - 2;
        window_draw_helper.drawFilledRect(position, size, note_color);
    }

    // Draw inserted note, if any
    if (!note_edition)
    {
        return;
    }

    const int absolute_note_index
        = std::min(note_pressed_index, note_moving_index);

    position.x
        = absolute_note_index * (CELL_WIDTH + CELL_SPACING) - offset.x + 1;

    position.y = ((6 * 12 - int(note_pressed_frequency)) - 1)
            * (CELL_HEIGHT + CELL_SPACING)
        - offset.y + 1;

    size.x = (CELL_WIDTH + CELL_SPACING)
            * (std::abs(note_moving_index - note_pressed_index) + 1)
        - 2;

    window_draw_helper.drawFilledRect(position, size, ImColor { 0xFF, 0, 0 });
}

float SequencerView::viewWidth()
{
    return (CELL_WIDTH + CELL_SPACING) * NOTES_BY_PHRASE * PHRASE_COUNT;
}

}
