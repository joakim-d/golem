#include "app.h"

#include "keys.h"

#include "golem/song_text.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <utility>

namespace golem::editor {

namespace {

    constexpr int kPageRows = 16;
    constexpr int kBeatRows = 4;
    const SDL_DialogFileFilter kSongFilter = {"Golem songs", "gsong"};

    std::string hex(
        unsigned value,
        int digits)
    {
        char text[8];
        std::snprintf(text, sizeof text, "%0*X", digits, value);
        return text;
    }

    // The five fields of a cell as shown in the grid: "C-4", "1", "C", "0", "F".
    std::string field_text(
        const Cell& cell,
        edit::Column column)
    {
        const bool has_effect = cell.effect != 0 || cell.param != 0;
        switch (column) {
        case edit::Column::Note:
            return note_name(cell.note);
        case edit::Column::Instrument:
            return cell.instrument != 0 ? hex(cell.instrument, 1) : ".";
        case edit::Column::Effect:
            return has_effect ? hex(cell.effect, 1) : ".";
        case edit::Column::ParamHigh:
            return has_effect ? hex(cell.param >> 4, 1) : ".";
        case edit::Column::ParamLow:
            return has_effect ? hex(cell.param & 0x0F, 1) : ".";
        }
        return {};
    }

    bool command_key(SDL_Keymod mod)
    {
        return (mod & (SDL_KMOD_CTRL | SDL_KMOD_GUI)) != 0;
    }

} // namespace

App::App(SDL_Window* window)
    : window_(window)
{
}

void App::open(const std::string& path)
{
    try {
        doc_ = edit::Document::open(path);
        player_.stop();
        scroll_to_cursor_ = true;
    } catch (const std::exception& e) {
        fail("Cannot open " + path + ":\n" + e.what());
    }
}

void App::play()
{
    try {
        player_.play(doc_.song());
    } catch (const std::exception& e) {
        fail(std::string("Cannot play the song:\n") + e.what());
    }
}

bool App::quit_requested() const
{
    return quit_;
}

LivePlayer& App::player()
{
    return player_;
}

// --- Actions ---

void App::request(Pending action)
{
    if (doc_.modified()) {
        pending_ = action;
        ask_unsaved_ = true;
    } else {
        run(action);
    }
}

void App::run(Pending action)
{
    switch (action) {
    case Pending::New:
        player_.stop();
        doc_ = edit::Document();
        scroll_to_cursor_ = true;
        break;
    case Pending::Open:
        show_dialog(Dialog::Open);
        break;
    case Pending::Quit:
        quit_ = true;
        break;
    case Pending::None:
        break;
    }
}

void App::save()
{
    if (doc_.path().empty()) {
        save_as();
        return;
    }
    try {
        doc_.save();
    } catch (const std::exception& e) {
        fail(std::string("Cannot save:\n") + e.what());
    }
}

void App::save_as()
{
    show_dialog(Dialog::SaveAs);
}

void App::show_dialog(Dialog dialog)
{
    {
        // SDL's callback does not say which dialog answers: remember it first, since the
        // answer may come before SDL_Show*FileDialog returns.
        const std::lock_guard<std::mutex> lock(dialog_mutex_);
        dialog_result_ = std::make_pair(dialog, std::string());
    }
    // The answer comes back through on_dialog(); `this` outlives the window.
    if (dialog == Dialog::Open) {
        SDL_ShowOpenFileDialog(&App::on_dialog, this, window_, &kSongFilter, 1, nullptr, false);
    } else {
        const std::string location = doc_.path().empty() ? "song.gsong" : doc_.path();
        SDL_ShowSaveFileDialog(&App::on_dialog, this, window_, &kSongFilter, 1, location.c_str());
    }
}

void SDLCALL App::on_dialog(
    void* userdata,
    const char* const* files,
    int)
{
    auto* app = static_cast<App*>(userdata);
    const std::lock_guard<std::mutex> lock(app->dialog_mutex_);
    if (!app->dialog_result_) {
        return;
    }
    if (files == nullptr || files[0] == nullptr) {
        app->dialog_result_.reset(); // Cancelled, or an error.
        return;
    }
    app->dialog_result_->second = files[0];
}

void App::toggle_playback()
{
    if (player_.is_playing()) {
        player_.stop();
    } else {
        play();
    }
}

void App::fail(const std::string& message)
{
    error_ = message;
}

// --- Keyboard ---

void App::handle_event(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        request(Pending::Quit);
        return;
    }
    if (event.type != SDL_EVENT_KEY_DOWN) {
        return;
    }
    if (handle_shortcut(event.key)) {
        return;
    }
    if (pattern_focused_ && !ImGui::GetIO().WantTextInput) {
        handle_pattern_key(event.key);
    }
}

bool App::handle_shortcut(const SDL_KeyboardEvent& key)
{
    if (!command_key(key.mod)) {
        return false;
    }
    const bool shift = (key.mod & SDL_KMOD_SHIFT) != 0;
    switch (key.scancode) {
    case SDL_SCANCODE_N:
        request(Pending::New);
        return true;
    case SDL_SCANCODE_O:
        request(Pending::Open);
        return true;
    case SDL_SCANCODE_S:
        shift ? save_as() : save();
        return true;
    case SDL_SCANCODE_Q:
        request(Pending::Quit);
        return true;
    default:
        break;
    }
    // Undo and redo follow the layout's letters, as in other applications.
    if (key.key == SDLK_Z) {
        shift ? doc_.redo() : doc_.undo();
        scroll_to_cursor_ = true;
        return true;
    }
    if (key.key == SDLK_Y) {
        doc_.redo();
        scroll_to_cursor_ = true;
        return true;
    }
    return false;
}

void App::handle_pattern_key(const SDL_KeyboardEvent& key)
{
    scroll_to_cursor_ = true;
    switch (key.scancode) {
    case SDL_SCANCODE_UP:
        doc_.move_rows(-1);
        return;
    case SDL_SCANCODE_DOWN:
        doc_.move_rows(1);
        return;
    case SDL_SCANCODE_LEFT:
        doc_.move_columns(-1);
        return;
    case SDL_SCANCODE_RIGHT:
        doc_.move_columns(1);
        return;
    case SDL_SCANCODE_PAGEUP:
        doc_.move_rows(-kPageRows);
        return;
    case SDL_SCANCODE_PAGEDOWN:
        doc_.move_rows(kPageRows);
        return;
    case SDL_SCANCODE_HOME:
        doc_.move_rows(-static_cast<int>(kRowsPerPattern));
        return;
    case SDL_SCANCODE_END:
        doc_.move_rows(static_cast<int>(kRowsPerPattern));
        return;
    case SDL_SCANCODE_TAB:
        doc_.move_channels((key.mod & SDL_KMOD_SHIFT) != 0 ? -1 : 1);
        return;
    case SDL_SCANCODE_DELETE:
    case SDL_SCANCODE_BACKSPACE:
        doc_.clear();
        return;
    case SDL_SCANCODE_SPACE:
        toggle_playback();
        return;
    default:
        break;
    }
    if (doc_.cursor().column == edit::Column::Note) {
        if (key.scancode == SDL_SCANCODE_1) {
            doc_.enter_note_off();
            return;
        }
        if (const auto c = qwerty_char(key.scancode)) {
            doc_.enter_key(*c); // Keys that are not notes change nothing.
        }
        return;
    }
    if (const auto digit = hex_value(key.scancode, key.key)) {
        doc_.enter_hex(*digit);
    }
}

// --- Drawing ---

void App::draw()
{
    {
        // Apply a file dialog's choice, made on SDL's side.
        std::optional<std::pair<Dialog, std::string>> chosen;
        {
            const std::lock_guard<std::mutex> lock(dialog_mutex_);
            if (dialog_result_ && !dialog_result_->second.empty()) {
                chosen = std::move(dialog_result_);
                dialog_result_.reset();
            }
        }
        if (chosen && chosen->first == Dialog::Open) {
            open(chosen->second);
        } else if (chosen) {
            std::filesystem::path path(chosen->second);
            if (path.extension().empty()) {
                path += ".gsong";
            }
            try {
                doc_.save_as(path.string());
            } catch (const std::exception& e) {
                fail(std::string("Cannot save:\n") + e.what());
            }
        }
    }

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    draw_menu();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::Begin(
        "Golem",
        nullptr,
        ImGuiWindowFlags_NoDecoration
            | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoBringToFrontOnFocus);
    draw_toolbar();
    ImGui::Separator();
    draw_orders();
    ImGui::SameLine();
    draw_pattern();
    ImGui::End();
    draw_popups();
    update_title();
}

void App::draw_menu()
{
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) {
            request(Pending::New);
        }
        if (ImGui::MenuItem("Open...", "Ctrl+O")) {
            request(Pending::Open);
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            save();
        }
        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
            save_as();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Quit", "Ctrl+Q")) {
            request(Pending::Quit);
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z", false, doc_.can_undo())) {
            doc_.undo();
        }
        if (ImGui::MenuItem("Redo", "Ctrl+Y", false, doc_.can_redo())) {
            doc_.redo();
        }
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void App::draw_toolbar()
{
    const std::string unavailable = playback_unavailable_reason();
    ImGui::BeginDisabled(!unavailable.empty());
    if (ImGui::Button(player_.is_playing() ? "Stop" : "Play", ImVec2(80, 0))) {
        toggle_playback();
    }
    ImGui::EndDisabled();
    if (!unavailable.empty()) {
        ImGui::SetItemTooltip("Playback is unavailable: %s", unavailable.c_str());
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(90);
    int octave = doc_.octave();
    if (ImGui::SliderInt("Octave", &octave, edit::kLowestOctave, edit::kHighestOctave)) {
        doc_.set_octave(octave);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90);
    int step = doc_.edit_step();
    if (ImGui::SliderInt("Edit step", &step, 0, 16)) {
        doc_.set_edit_step(step);
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    int instrument = doc_.instrument();
    if (ImGui::InputInt("Instrument", &instrument)) {
        doc_.set_instrument(static_cast<std::uint8_t>(std::clamp(instrument, 1, 15)));
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(60);
    // Edited in place, then applied as one change (one undo step) when the field is left.
    if (!ticks_editing_) {
        ticks_edit_ = doc_.song().ticks_per_row;
    }
    ImGui::InputScalar("Ticks per row", ImGuiDataType_U8, &ticks_edit_);
    ticks_editing_ = ImGui::IsItemActive();
    if (ImGui::IsItemDeactivatedAfterEdit() && ticks_edit_ != doc_.song().ticks_per_row) {
        doc_.set_ticks_per_row(ticks_edit_);
    }
}

void App::draw_orders()
{
    ImGui::BeginChild("Orders", ImVec2(230, 0), ImGuiChildFlags_Borders);
    ImGui::TextUnformatted("Orders");
    if (ImGui::Button("Insert")) {
        doc_.insert_order();
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove")) {
        doc_.remove_order();
    }
    ImGui::SetNextItemWidth(60);
    std::uint8_t pattern = doc_.pattern();
    if (ImGui::InputScalar(
            "Pattern",
            ImGuiDataType_U8,
            &pattern,
            nullptr,
            nullptr,
            "%02X",
            ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_EnterReturnsTrue)) {
        doc_.set_pattern(pattern);
    }
    ImGui::SetItemTooltip("Pattern of channel %d in this order", int(doc_.cursor().channel) + 1);
    ImGui::SameLine();
    if (ImGui::Button("New")) {
        doc_.new_pattern();
    }
    ImGui::SetItemTooltip("A new empty pattern for this channel");
    ImGui::Separator();
    const auto& orders = doc_.song().orders;
    for (std::size_t i = 0; i < orders.size(); ++i) {
        const auto& order = orders[i];
        const std::string label = hex(unsigned(i), 2)
                                + "   "
                                + hex(order[0], 2)
                                + " "
                                + hex(order[1], 2)
                                + " "
                                + hex(order[2], 2)
                                + " "
                                + hex(order[3], 2)
                                + "##order"
                                + std::to_string(i);
        if (ImGui::Selectable(label.c_str(), i == doc_.cursor().order)) {
            doc_.set_order(i);
        }
    }
    ImGui::EndChild();
}

void App::draw_pattern()
{
    ImGui::BeginChild("Pattern", ImVec2(0, 0), ImGuiChildFlags_Borders);
    pattern_focused_ = ImGui::IsWindowFocused();
    const auto& cursor = doc_.cursor();
    const auto& order = doc_.song().orders[cursor.order];

    if (ImGui::BeginTable("Rows", 1 + int(kChannels), ImGuiTableFlags_ScrollY)) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Row", ImGuiTableColumnFlags_WidthFixed, 30);
        for (std::size_t channel = 0; channel < kChannels; ++channel) {
            const std::string name =
                "Ch" + std::to_string(channel + 1) + " (" + hex(order[channel], 2) + ")";
            ImGui::TableSetupColumn(name.c_str());
        }
        ImGui::TableHeadersRow();

        for (std::size_t row = 0; row < kRowsPerPattern; ++row) {
            ImGui::TableNextRow();
            if (row % kBeatRows == 0) {
                ImGui::TableSetBgColor(
                    ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_TableRowBgAlt));
            }
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(hex(unsigned(row), 2).c_str());
            if (row == cursor.row && scroll_to_cursor_) {
                ImGui::SetScrollHereY();
                scroll_to_cursor_ = false;
            }
            for (std::size_t channel = 0; channel < kChannels; ++channel) {
                ImGui::TableNextColumn();
                const Cell& cell = doc_.song().patterns[order[channel]][row];
                for (int c = 0; c < edit::kColumnsPerCell; ++c) {
                    const auto column = static_cast<edit::Column>(c);
                    const std::string text = field_text(cell, column);
                    const bool selected =
                        row == cursor.row && channel == cursor.channel && column == cursor.column;
                    ImGui::PushID(int((row * kChannels + channel) * edit::kColumnsPerCell) + c);
                    if (c > 0) {
                        ImGui::SameLine(0, c == 1 || c == 2 ? 8.0F : 0.0F);
                    }
                    const ImVec2 size = ImGui::CalcTextSize(text.c_str());
                    if (ImGui::Selectable(
                            text.c_str(), selected, ImGuiSelectableFlags_None, size)) {
                        doc_.set_cursor(edit::Cursor {cursor.order, row, channel, column});
                    }
                    ImGui::PopID();
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::EndChild();
}

void App::draw_popups()
{
    if (ask_unsaved_) {
        ImGui::OpenPopup("Unsaved changes");
        ask_unsaved_ = false;
    }
    if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("The song has unsaved changes.");
        if (ImGui::Button("Save")) {
            save();
            ImGui::CloseCurrentPopup();
            pending_ = Pending::None; // Saving may open a dialog: let the user retry after it.
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard")) {
            ImGui::CloseCurrentPopup();
            run(std::exchange(pending_, Pending::None));
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            pending_ = Pending::None;
        }
        ImGui::EndPopup();
    }

    if (!error_.empty()) {
        ImGui::OpenPopup("Error");
    }
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted(error_.c_str());
        if (ImGui::Button("OK")) {
            error_.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void App::update_title()
{
    const std::string name =
        doc_.path().empty() ? "Untitled" : std::filesystem::path(doc_.path()).filename().string();
    const std::string title = name + (doc_.modified() ? " *" : "") + " - Golem";
    if (title != title_) {
        title_ = title;
        SDL_SetWindowTitle(window_, title_.c_str());
    }
}

} // namespace golem::editor
