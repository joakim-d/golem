#include <interfaces/PianoRollView.h>

#include <golem/use_cases/ProjectUseCases.h>
#include <golem/utils/scope_guard.h>

#include <interfaces/ImGuiTools.h>

#include <imgui.h>

namespace gui
{

namespace
{
    constexpr size_t OCTAVE_COUNT = 6;
    constexpr size_t NOTE_COUNT = 12;
    constexpr size_t WHITE_KEY_HEIGHT = 23;
    constexpr size_t BLACK_KEY_HEIGHT = 14;
    constexpr size_t SPACING = 1;

    static const bool IS_WHITE_LUT[12] { true,  false, true,  false,
                                         true,  false, true,  true,
                                         false, true,  false, true };

}

static ImVec2* initKeyRectanglePosition()
{
    static ImVec2 positions[12 * 6];

    const int white_key_indexes[7] = { 0, 2, 4, 6, 7, 9, 11 };
    const int black_key_indexes[5] = { 1, 3, 5, 8, 10 };

    const int white_key_width = 64;
    const int black_key_width = 46;

    for (int octave = 0; octave < 6; octave++)
    {
        int i = 0;
        for (auto white_key_index : white_key_indexes)
        {
            positions[octave * 12 + white_key_index].x = 0;
            positions[octave * 12 + white_key_index].y
                = (168 * octave) + i * (WHITE_KEY_HEIGHT + SPACING);
            i++;
        }
        for (auto black_key_index : black_key_indexes)
        {
            positions[octave * 12 + black_key_index].x = 0;
            positions[octave * 12 + black_key_index].y
                = (168 * octave) + black_key_index * 14;
        }
    }

    return positions;
}

static ImVec2* initKeyRectangleSize()
{
    static ImVec2 size[12 * 6];

    const int white_key_indexes[7] = { 0, 2, 4, 6, 7, 9, 11 };
    const int black_key_indexes[5] = { 1, 3, 5, 8, 10 };

    const int white_key_width = 64;
    const int black_key_width = 46;

    for (int octave = 0; octave < 6; octave++)
    {
        int i = 0;
        for (auto white_key_index : white_key_indexes)
        {
            size[octave * 12 + white_key_index].x = white_key_width;
            size[octave * 12 + white_key_index].y = WHITE_KEY_HEIGHT;
            i++;
        }
        for (auto black_key_index : black_key_indexes)
        {
            size[octave * 12 + black_key_index].x = black_key_width;
            size[octave * 12 + black_key_index].y = BLACK_KEY_HEIGHT;
        }
    }

    return size;
}

PianoRollView::PianoRollView(
    use_cases::ProjectUseCases& project_use_cases,
    float y_offset)
{
    ImGui::BeginChild(
        "PianoRollView", ImVec2 { 64, -1 }, 0, ImGuiWindowFlags_NoScrollbar);

    utils::ScopeGuard scope_guard { ImGui::EndChild };

    static ImVec2* KEY_RECTANGLE_POSITIONS = initKeyRectanglePosition();
    static ImVec2* KEY_RECTANGLE_SIZES = initKeyRectangleSize();

    const int black_key_height = 14;
    const int white_key_width = 64;
    const int black_key_width = 46;

    ImVec2 rect_position;
    const ImVec2 rect_size {};

    const auto window_size = ImGui::GetWindowSize();

    const int key_index_begin = (y_offset / (BLACK_KEY_HEIGHT));
    const int max_keys_to_draw
        = (window_size.y / BLACK_KEY_HEIGHT) + BLACK_KEY_HEIGHT;
    const int keys_count_to_draw
        = key_index_begin + max_keys_to_draw > NOTE_COUNT * OCTAVE_COUNT
        ? NOTE_COUNT * OCTAVE_COUNT - key_index_begin
        : max_keys_to_draw;

    int index = 0;

    auto black_color = ImColor { 0x00, 0x00, 0x00 };
    auto white_color = ImColor { 0xFF, 0xFF, 0xFF };

    ImVec2 current_key_position;
    ImVec2 current_key_size;
    current_key_position.x = 0;

    WindowDrawHelper draw_helper;

    for (int i = key_index_begin; i < key_index_begin + keys_count_to_draw; ++i)
    {
        if (!IS_WHITE_LUT[i % 12])
            continue;

        current_key_position.y = KEY_RECTANGLE_POSITIONS[i].y - y_offset;
        current_key_size.x = KEY_RECTANGLE_SIZES[i].x;
        current_key_size.y = KEY_RECTANGLE_SIZES[i].y;
        draw_helper.drawFilledRect(
            current_key_position, current_key_size, white_color);

        if (i % 12 == 11)
        {
            unsigned octave = 7 - (i / 12);
            char text[3];
            snprintf(text, sizeof(text), "C%u", octave);
            current_key_position.y += 10;
            draw_helper.addText(current_key_position, black_color, text);
        }
    }

    for (int i = key_index_begin; i < key_index_begin + keys_count_to_draw; ++i)
    {
        if (IS_WHITE_LUT[i % 12])
            continue;
        current_key_position.y = KEY_RECTANGLE_POSITIONS[i].y - y_offset;
        current_key_size.x = KEY_RECTANGLE_SIZES[i].x;
        current_key_size.y = KEY_RECTANGLE_SIZES[i].y;
        draw_helper.drawFilledRect(
            current_key_position, current_key_size, black_color);
    }

    ImGui::Dummy(ImVec2 { 64, -1 });
}

float PianoRollView::viewHeight()
{
    return 168 * OCTAVE_COUNT; // 7 white keys by octave * height
}

float PianoRollView::viewWidth()
{
    return 64;
}

}
