#include "app.h"

#include "keys.h"

#include "golem/instrument_fields.h"
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
    constexpr Uint64 kMaxPreviewMs = 3000;
    const ImU32 kPlayingRowColor = IM_COL32(60, 140, 90, 110);
    constexpr float kSidePanelWidth = 310;
    constexpr float kWaveBarWidth = 8; // Wave canvas: one bar per sample...
    constexpr float kWaveStepHeight = 8; // ...and this much per value step.
    const char* const kDuties[] = {"12.5%", "25%", "50%", "75%"};
    const char* const kWaveVolumes[] = {"Mute", "100%", "50%", "25%"};
    const char* const kEnvelopeDirections[] = {"Down", "Up"};
    const char* const kLfsrWidths[] = {"15-bit (noise)", "7-bit (metallic)"};
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

void App::render_audio(
    StereoSample* samples,
    std::size_t count)
{
    player_.render(samples, count);
    static thread_local std::vector<StereoSample> preview;
    preview.resize(count);
    preview_.render(preview.data(), count);
    mix_into(samples, preview.data(), count);
}

void App::set_audio_latency(std::size_t samples)
{
    audio_latency_ = samples;
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

void App::start_preview(
    std::uint8_t note,
    SDL_Scancode key)
{
    if (!playback_unavailable_reason().empty()) {
        return;
    }
    try {
        preview_.play(
            edit::preview_song(doc_.song(), doc_.cursor().channel, note, doc_.instrument()));
        preview_key_ = key;
        preview_started_ = SDL_GetTicks();
    } catch (const std::exception&) {
        stop_preview(); // A preview is a courtesy: entering the note is what matters.
    }
}

void App::stop_preview()
{
    preview_.stop();
    preview_key_.reset();
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
    if (event.type == SDL_EVENT_KEY_UP && preview_key_ == event.key.scancode) {
        stop_preview();
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
            const auto note = edit::note_for_key(*c, doc_.octave());
            doc_.enter_key(*c); // Keys that are not notes change nothing.
            if (note && !key.repeat) {
                start_preview(*note, key.scancode);
            }
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

    if (preview_key_ && SDL_GetTicks() - preview_started_ > kMaxPreviewMs) {
        stop_preview();
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
    draw_side_panel();
    ImGui::SameLine();
    draw_pattern();
    ImGui::End();
    draw_popups();
    if (!ImGui::IsAnyItemActive()) {
        doc_.finish_edit(); // A slider drag or a wave drawing ends with the mouse button.
    }
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
    ImGui::Checkbox("Follow", &follow_);
    ImGui::SetItemTooltip("While playing, show the playing order and scroll with its rows");

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

void App::draw_side_panel()
{
    ImGui::BeginChild("Side", ImVec2(kSidePanelWidth, 0), ImGuiChildFlags_Borders);
    if (ImGui::BeginTabBar("SideTabs")) {
        const auto tab = [this](const char* label, SideTab which) {
            const bool select = select_tab_ == which;
            return ImGui::BeginTabItem(
                label, nullptr, select ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None);
        };
        if (tab("Orders", SideTab::Orders)) {
            draw_orders();
            ImGui::EndTabItem();
        }
        if (tab("Instruments", SideTab::Instruments)) {
            draw_instruments();
            ImGui::EndTabItem();
        }
        if (tab("Waves", SideTab::Waves)) {
            draw_waves();
            ImGui::EndTabItem();
        }
        select_tab_.reset();
        ImGui::EndTabBar();
    }
    ImGui::EndChild();
}

void App::draw_orders()
{
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
}

// --- Instruments and waves ---

void App::draw_instruments()
{
    int type = static_cast<int>(instrument_type_);
    ImGui::RadioButton("Pulse", &type, static_cast<int>(InstrumentType::Pulse));
    ImGui::SameLine();
    ImGui::RadioButton("Wave", &type, static_cast<int>(InstrumentType::Wave));
    ImGui::SameLine();
    ImGui::RadioButton("Noise", &type, static_cast<int>(InstrumentType::Noise));
    instrument_type_ = static_cast<InstrumentType>(type);
    ImGui::TextDisabled("Ch1-2 play pulse, Ch3 wave, Ch4 noise");

    ImGui::SetNextItemWidth(100);
    int instrument = doc_.instrument();
    if (ImGui::InputInt("Instrument", &instrument)) {
        doc_.set_instrument(static_cast<std::uint8_t>(std::clamp(instrument, 1, 15)));
    }
    ImGui::SetItemTooltip("Also the instrument given to the notes you enter");
    ImGui::Separator();

    ImGui::PushItemWidth(150);
    switch (instrument_type_) {
    case InstrumentType::Pulse:
        draw_pulse_instrument();
        break;
    case InstrumentType::Wave:
        draw_wave_instrument();
        break;
    case InstrumentType::Noise:
        draw_noise_instrument();
        break;
    }
    ImGui::PopItemWidth();
}

namespace {

    // The envelope widgets shared by pulse and noise instruments; true when one changed.
    bool envelope_widgets(
        int& volume,
        bool& increase,
        int& pace)
    {
        bool changed = ImGui::SliderInt("Volume", &volume, 0, 15);
        int direction = increase ? 1 : 0;
        if (ImGui::Combo("Envelope", &direction, kEnvelopeDirections, 2)) {
            increase = direction == 1;
            changed = true;
        }
        changed |= ImGui::SliderInt("Envelope pace", &pace, 0, 7);
        ImGui::SetItemTooltip("Frames of 1/64 s per volume step; 0 holds the volume");
        return changed;
    }

    // Length enable and timer; the note lasts (max + 1 - timer) / 256 s.
    bool length_widgets(
        bool& enable,
        int& length,
        int max)
    {
        bool changed = ImGui::Checkbox("Length", &enable);
        ImGui::BeginDisabled(!enable);
        changed |= ImGui::SliderInt("Length timer", &length, 0, max);
        ImGui::SameLine();
        ImGui::TextDisabled("%.0f ms", (max + 1 - length) * 1000.0 / 256.0);
        ImGui::EndDisabled();
        return changed;
    }

} // namespace

void App::draw_pulse_instrument()
{
    const std::uint8_t number = doc_.instrument();
    edit::PulseFields fields = edit::pulse_fields(doc_.song().pulse_instruments[number - 1]);
    bool changed = ImGui::Combo("Duty", &fields.duty, kDuties, 4);
    changed |= envelope_widgets(fields.volume, fields.envelope_increase, fields.envelope_pace);
    changed |= length_widgets(fields.length_enable, fields.length, 63);
    ImGui::SeparatorText("Sweep (channel 1 only)");
    changed |= ImGui::SliderInt("Sweep pace", &fields.sweep_pace, 0, 7);
    ImGui::SetItemTooltip("Frames of 1/128 s per pitch step; 0 is no sweep");
    changed |= ImGui::Checkbox("Sweep down", &fields.sweep_decrease);
    changed |= ImGui::SliderInt("Sweep steps", &fields.sweep_steps, 0, 7);
    ImGui::SetItemTooltip("Each step moves the period by 1/2^steps of itself: higher is finer");
    if (changed) {
        doc_.set_pulse_instrument(number, edit::pulse_instrument(fields));
    }
}

void App::draw_wave_instrument()
{
    const std::uint8_t number = doc_.instrument();
    edit::WaveFields fields = edit::wave_fields(doc_.song().wave_instruments[number - 1]);
    bool changed = ImGui::Combo("Volume", &fields.volume, kWaveVolumes, 4);
    changed |= ImGui::SliderInt("Wave", &fields.wave, 0, int(kWaves) - 1);
    ImGui::SameLine();
    if (ImGui::SmallButton("Edit")) {
        wave_ = fields.wave;
        select_tab_ = SideTab::Waves;
    }
    changed |= length_widgets(fields.length_enable, fields.length, 255);
    if (changed) {
        doc_.set_wave_instrument(number, edit::wave_instrument(fields));
    }
}

void App::draw_noise_instrument()
{
    const std::uint8_t number = doc_.instrument();
    edit::NoiseFields fields = edit::noise_fields(doc_.song().noise_instruments[number - 1]);
    int lfsr = fields.short_lfsr ? 1 : 0;
    bool changed = false;
    if (ImGui::Combo("LFSR", &lfsr, kLfsrWidths, 2)) {
        fields.short_lfsr = lfsr == 1;
        changed = true;
    }
    changed |= envelope_widgets(fields.volume, fields.envelope_increase, fields.envelope_pace);
    changed |= length_widgets(fields.length_enable, fields.length, 63);
    if (changed) {
        doc_.set_noise_instrument(number, edit::noise_instrument(fields));
    }
}

void App::draw_waves()
{
    ImGui::SetNextItemWidth(150);
    ImGui::SliderInt("Wave", &wave_, 0, int(kWaves) - 1);
    const auto wave = static_cast<std::uint8_t>(wave_);
    draw_wave_canvas();

    // The wave as hex, edited in place and applied with Enter.
    if (!wave_hex_editing_) {
        const std::string text = edit::wave_hex(doc_.song().waves[wave]);
        std::snprintf(wave_hex_edit_, sizeof wave_hex_edit_, "%s", text.c_str());
    }
    ImGui::SetNextItemWidth(kSidePanelWidth - 70);
    if (ImGui::InputText(
            "##hex",
            wave_hex_edit_,
            sizeof wave_hex_edit_,
            ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue)) {
        const auto parsed = edit::parse_wave_hex(wave_hex_edit_);
        wave_hex_invalid_ = !parsed;
        if (parsed) {
            doc_.set_wave(wave, *parsed);
        }
    }
    wave_hex_editing_ = ImGui::IsItemActive();
    ImGui::SetItemTooltip("32 hex digits, one per sample; Enter applies them");
    ImGui::SameLine();
    if (ImGui::Button("Copy")) {
        ImGui::SetClipboardText(edit::wave_hex(doc_.song().waves[wave]).c_str());
    }
    if (wave_hex_invalid_) {
        ImGui::TextColored(ImVec4(1, 0.4F, 0.4F, 1), "Needs exactly 32 hex digits");
    }

    ImGui::TextUnformatted("Presets");
    const std::pair<const char*, edit::WavePreset> presets[] = {
        {"Square", edit::WavePreset::Square},
        {"Saw", edit::WavePreset::Saw},
        {"Triangle", edit::WavePreset::Triangle},
        {"Sine", edit::WavePreset::Sine},
    };
    for (const auto& [label, preset] : presets) {
        if (label != presets[0].first) {
            ImGui::SameLine();
        }
        if (ImGui::Button(label)) {
            doc_.set_wave(wave, edit::wave_preset(preset));
        }
    }
}

void App::draw_wave_canvas()
{
    const auto wave = static_cast<std::uint8_t>(wave_);
    const ImVec2 size(kWaveBarWidth * edit::kWaveSamples, kWaveStepHeight * 16);
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("canvas", size);
    ImGui::SetItemTooltip("Click or drag to draw the wave");

    if (ImGui::IsItemActive()) {
        const ImVec2 mouse = ImGui::GetIO().MousePos;
        const int sample = std::clamp(
            static_cast<int>((mouse.x - origin.x) / kWaveBarWidth), 0, int(edit::kWaveSamples) - 1);
        const int value =
            std::clamp(15 - static_cast<int>((mouse.y - origin.y) / kWaveStepHeight), 0, 15);
        // Fill the samples between this frame and the last, for a quick stroke.
        const auto [from, from_value] = last_drawn_.value_or(std::make_pair(sample, value));
        const int steps = std::abs(sample - from);
        for (int i = 0; i <= steps; ++i) {
            const int at = from + (sample > from ? i : -i);
            const int at_value = steps == 0 ? value : from_value + (value - from_value) * i / steps;
            doc_.set_wave_sample(
                wave, static_cast<std::size_t>(at), static_cast<std::uint8_t>(at_value));
        }
        last_drawn_ = std::make_pair(sample, value);
    } else {
        last_drawn_.reset();
    }

    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImVec2 end(origin.x + size.x, origin.y + size.y);
    draw->AddRectFilled(origin, end, ImGui::GetColorU32(ImGuiCol_FrameBg));
    const ImU32 bar = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
    for (std::size_t i = 0; i < edit::kWaveSamples; ++i) {
        const int value = edit::wave_sample(doc_.song().waves[wave], i);
        const float x = origin.x + kWaveBarWidth * static_cast<float>(i);
        const float top = origin.y + kWaveStepHeight * static_cast<float>(15 - value);
        draw->AddRectFilled(ImVec2(x + 1, top), ImVec2(x + kWaveBarWidth - 1, end.y), bar);
    }
    draw->AddRect(origin, end, ImGui::GetColorU32(ImGuiCol_Border));
}

void App::draw_pattern()
{
    ImGui::BeginChild("Pattern", ImVec2(0, 0), ImGuiChildFlags_Borders);
    // The table's rows scroll in a child window of their own, which a click focuses.
    pattern_focused_ = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

    // The row being heard, and Follow: show its order, and scroll when it changes.
    const auto playing = player_.position(audio_latency_);
    bool scroll_to_playing = false;
    if (playing && follow_ && playing->order < doc_.song().orders.size()) {
        if (playing->order != doc_.cursor().order) {
            doc_.set_order(playing->order);
        }
        const auto heard = std::make_pair(playing->order, playing->row);
        scroll_to_playing = followed_row_ != heard;
        followed_row_ = heard;
    } else {
        followed_row_.reset();
    }

    const auto& cursor = doc_.cursor();
    const auto& order = doc_.song().orders[cursor.order];
    const bool shows_playing_order = playing && playing->order == cursor.order;

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
            const bool is_playing_row = shows_playing_order && row == playing->row;
            if (is_playing_row) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, kPlayingRowColor);
            }
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(hex(unsigned(row), 2).c_str());
            if (row == cursor.row && scroll_to_cursor_) {
                ImGui::SetScrollHereY();
                scroll_to_cursor_ = false;
            }
            if (is_playing_row && scroll_to_playing) {
                ImGui::SetScrollHereY();
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
