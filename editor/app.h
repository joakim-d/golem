#pragma once

#include "golem/edit.h"
#include "golem/live_player.h"

#include <SDL3/SDL.h>

#include <atomic>
#include <mutex>
#include <optional>
#include <string>

namespace golem::editor {

// The editor window's contents and behaviour. Song logic lives in golem::edit::Document and
// playback in golem::LivePlayer; this class maps keys, widgets and dialogs onto them.
class App {
public:
    explicit App(SDL_Window* window);

    // Opens a song; a failure is shown in the window.
    void open(const std::string& path);
    void play();

    // Keyboard: shortcuts, and pattern entry while the pattern has the focus.
    void handle_event(const SDL_Event& event);
    // Draws one frame of the UI (between ImGui::NewFrame and ImGui::Render).
    void draw();

    bool quit_requested() const;

    // Fills `samples` with the song and the note preview mixed; runs on SDL's audio thread.
    void render_audio(
        StereoSample* samples,
        std::size_t count);
    // Samples rendered but not heard yet, for the playing row.
    void set_audio_latency(std::size_t samples);

private:
    enum class Pending {
        None,
        New,
        Open,
        Quit,
    };

    enum class Dialog {
        Open,
        SaveAs,
    };

    enum class SideTab {
        Orders,
        Instruments,
        Waves,
    };

    enum class InstrumentType {
        Pulse,
        Wave,
        Noise,
    };

    // Runs `action` now, or after asking about unsaved changes.
    void request(Pending action);
    void run(Pending action);
    void save();
    void save_as();
    void show_dialog(Dialog dialog);
    static void SDLCALL on_dialog(
        void* userdata,
        const char* const* files,
        int filter);
    void toggle_playback();
    // Plays `note` with the current instrument on the cursor's channel until `key` is released.
    void start_preview(
        std::uint8_t note,
        SDL_Scancode key);
    void stop_preview();
    void fail(const std::string& message);

    bool handle_shortcut(const SDL_KeyboardEvent& key);
    void handle_pattern_key(const SDL_KeyboardEvent& key);

    void draw_menu();
    void draw_toolbar();
    void draw_side_panel();
    void draw_orders();
    void draw_instruments();
    void draw_pulse_instrument();
    void draw_wave_instrument();
    void draw_noise_instrument();
    void draw_waves();
    void draw_wave_canvas();
    void draw_pattern();
    void draw_popups();
    void update_title();

    SDL_Window* window_;
    edit::Document doc_;
    LivePlayer player_;
    LivePlayer preview_;
    std::optional<SDL_Scancode> preview_key_;
    Uint64 preview_started_ = 0; // SDL_GetTicks() when the preview started.
    std::atomic<std::size_t> audio_latency_ {0};
    bool follow_ = true;
    std::optional<std::pair<std::size_t, std::size_t>> followed_row_; // Order and row.
    bool quit_ = false;
    bool pattern_focused_ = false;
    bool scroll_to_cursor_ = true;
    Pending pending_ = Pending::None;
    bool ask_unsaved_ = false;
    std::string error_;
    std::string title_;
    std::uint8_t ticks_edit_ = 6;
    bool ticks_editing_ = false;

    std::optional<SideTab> select_tab_; // Opened by the next frame, e.g. from a button.
    InstrumentType instrument_type_ = InstrumentType::Pulse;
    int wave_ = 0; // Shown in the Waves tab.
    std::optional<std::pair<int, int>> last_drawn_; // Sample and value under the mouse.
    char wave_hex_edit_[48] = {};
    bool wave_hex_editing_ = false;
    bool wave_hex_invalid_ = false;

    // Results of SDL's file dialogs, which may arrive on another thread.
    std::mutex dialog_mutex_;
    std::optional<std::pair<Dialog, std::string>> dialog_result_;
};

} // namespace golem::editor
