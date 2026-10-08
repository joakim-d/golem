#pragma once

#include "golem/edit.h"
#include "golem/live_player.h"

#include <SDL3/SDL.h>

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
    LivePlayer& player();

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
    void fail(const std::string& message);

    bool handle_shortcut(const SDL_KeyboardEvent& key);
    void handle_pattern_key(const SDL_KeyboardEvent& key);

    void draw_menu();
    void draw_toolbar();
    void draw_orders();
    void draw_pattern();
    void draw_popups();
    void update_title();

    SDL_Window* window_;
    edit::Document doc_;
    LivePlayer player_;
    bool quit_ = false;
    bool pattern_focused_ = false;
    bool scroll_to_cursor_ = true;
    Pending pending_ = Pending::None;
    bool ask_unsaved_ = false;
    std::string error_;
    std::string title_;
    std::uint8_t ticks_edit_ = 6;
    bool ticks_editing_ = false;

    // Results of SDL's file dialogs, which may arrive on another thread.
    std::mutex dialog_mutex_;
    std::optional<std::pair<Dialog, std::string>> dialog_result_;
};

} // namespace golem::editor
