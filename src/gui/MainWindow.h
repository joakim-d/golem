#pragma once

#include <optional>

extern "C" {
struct SDL_Window;
}

namespace model {
class Song;
}

namespace gui {

class MainWindow {
public:
    static std::optional<MainWindow> create(
        int width,
        int height,
        const char* title);

    int execute();

private:
    MainWindow() = default;

    SDL_Window* m_window;
};

}
