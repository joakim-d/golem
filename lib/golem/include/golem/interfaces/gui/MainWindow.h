#pragma once

#include <golem/domain/events/ProjectDomainEventProcessor.h>
#include <golem/use_cases/ProjectUseCases.h>
#include <optional>

extern "C"
{
    struct SDL_Window;
}

namespace domain
{
class Song;
}

namespace gui
{

class MainWindow
{
public:
    static std::optional<MainWindow>
    create(int width, int height, const char* title);

    int execute(
        use_cases::ProjectUseCases& use_cases,
        domain::events::ProjectDomainEventProcessor& event_processor);

private:
    MainWindow() = default;

    SDL_Window* m_window;
};

}
