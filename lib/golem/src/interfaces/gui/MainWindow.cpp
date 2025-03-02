#include <golem/interfaces/gui/MainWindow.h>

#include <interfaces/gui/MenuBar.h>
#include <interfaces/gui/PlayProgressionTracker.h>
#include <interfaces/gui/style/Style.h>
#include <interfaces/gui/views/ConfigurationView.h>
#include <interfaces/gui/views/MainView.h>
#include <interfaces/gui/views/SongView.h>

#include <interfaces/graphics/FontManager.h>
#include <interfaces/graphics/GraphicsFactory.h>
#include <interfaces/graphics/LabelFactory.h>
#include <interfaces/graphics/Widget.h>

#include <interfaces/ecs/EntityManager.h>
#include <interfaces/ecs/EntityMemoryPool.h>
#include <interfaces/ecs/SystemFactory.h>

#include <interfaces/ecs/components/AnchorComponent.h>
#include <interfaces/ecs/components/BorderComponent.h>
#include <interfaces/ecs/components/FillComponent.h>
#include <interfaces/ecs/components/PositionComponent.h>
#include <interfaces/ecs/components/SizeComponent.h>

#include <interfaces/ecs/systems/AnchoringSystem.h>
#include <interfaces/ecs/systems/RendererSystem.h>
#include <interfaces/ecs/systems/UpdaterSystem.h>
#include <interfaces/ecs/systems/UserInputSystem.h>

#include <core/song_loader/JsonSongLoader.h>
#include <core/song_saver/JsonSongSaver.h>

#include <golem/domain/Song.h>

#include <SDL2/SDL.h>

#include <iostream>

constexpr auto TEMP_SONG_FILE = ".song_tmp.json";

namespace gui
{

std::optional<MainWindow>
MainWindow::create(int width, int height, const char* title)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (window == nullptr)
    {
        std::cerr << "Failed to open window" << std::endl;
        return std::nullopt;
    }

    MainWindow main_window;
    main_window.m_window = window;

    return main_window;
}

int MainWindow::execute(
    use_cases::ProjectUseCases& use_cases,
    domain::events::ProjectDomainEventProcessor& event_processor)
{
    ecs::EntityMemoryPool pool { 1024 * 1024 };

    ecs::EntityManager manager { pool };

    auto renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

    graphics::GraphicsFactory graphics_factory { renderer, manager };

    // Create window widget
    auto window_widget
        = graphics_factory.createWidget("window").addSize(800, 0);

    const auto window_entity = window_widget.entity;

    auto menu_bar_widget
        = graphics_factory.createWidget("menu_bar")
              .addSize(0, 32)
              .anchorLeft(window_entity, ecs::Left)
              .anchorRight(window_entity, ecs::Right)
              .anchorTop(window_entity, ecs::Top)
              .addFill(graphics::core::Color::fromHexa("#333333"));

    gui::MenuBar menu_bar(graphics_factory, menu_bar_widget.entity);

    auto configuration_widget
        = graphics_factory.createWidget("configuration_widget")
              .anchorTop(menu_bar_widget.entity, ecs::Bottom)
              .anchorRight(window_entity, ecs::Right)
              .anchorBottom(window_entity, ecs::Bottom)
              .addSize(256, 0)
              .addZPosition(30)
              .addFill(graphics::core::Color::fromHexa("#333333"));

    gui::views::ConfigurationView configuration_view {
        graphics_factory, configuration_widget, use_cases, event_processor
    };

    auto song_view_widget
        = graphics_factory.createWidget("song_view")
              .addSize(0, 48 * 4 + 16 + 4) // 48 pixels per track + 16 pixels
                                           // for header + 4 pixels for spacing
              .anchorLeft(window_entity, ecs::Left)
              .anchorRight(configuration_widget.entity, ecs::Left, 1)
              .anchorTop(menu_bar_widget.entity, ecs::Bottom);

    gui::views::SongView song_view { graphics_factory, song_view_widget,
                                     use_cases };

    auto main_widget
        = graphics_factory.createWidget("main_widget")
              .anchorTop(song_view_widget.entity, ecs::Bottom, 1)
              .anchorLeft(window_entity, ecs::Left)
              .anchorRight(configuration_widget.entity, ecs::Left, 1)
              .anchorBottom(window_entity, ecs::Bottom);

    gui::views::MainView main_view { graphics_factory, main_widget, use_cases };

    auto handle = song_view.trackClicked.register_callback(
        [&main_view](size_t track) { main_view.setCurrentTrackIndex(track); });

    auto progression_tracker
        = std::make_shared<PlayProgressionTracker>(song_view, main_view);

    class PlaybackInfoHandler
        : public domain::events::IDomainEventHandler<domain::PlaybackInfo>
    {
    public:
        PlaybackInfoHandler(
            std::shared_ptr<PlayProgressionTracker> progression_tracker)
            : m_progression_tracker(progression_tracker)
        {
        }
        void handleEvent(domain::PlaybackInfo event) override
        {
            m_progression_tracker->onProgressMade(
                event.current_tick, event.current_note);
        }

    private:
        std::shared_ptr<PlayProgressionTracker> m_progression_tracker;
    };

    PlaybackInfoHandler handler { progression_tracker };
    event_processor.registerEventHandler(&handler);

    window_widget.onKeyPressed(
        [&use_cases](ecs::KeyBoardContext context)
        {
            if (context.key_pressed == ' ')
            {
                use_cases.get_playback_info.execute().stopped
                    ? use_cases.play_song.execute(0)
                    : use_cases.stop_song.execute();
            }
        });

    ecs::SystemFactory factory { pool, manager };

    int window_width;
    int window_height;

    SDL_GetWindowSize(m_window, &window_width, &window_height);

    auto user_input_system = factory.createSystem<ecs::UserInputSystem>(
        window_width, window_height);

    auto updater_system = factory.createSystem<ecs::Updater>();

    auto anchoring_system = factory.createSystem<ecs::AnchoringSystem>();

    auto renderer_system = factory.createSystem<ecs::Renderer>(
        renderer, graphics_factory.textureManager());

    const auto refresh_ms = 1000 / 60;

    while (!SDL_QuitRequested())
    {
        auto time_before_update = SDL_GetTicks();
        user_input_system.update();
        updater_system.update();
        anchoring_system.update();

        renderer_system.update();

        auto sleep_time = std::clamp(
            int(refresh_ms - (SDL_GetTicks() - time_before_update)), 0,
            refresh_ms);

        SDL_Delay(sleep_time);
    }

    SDL_DestroyWindow(m_window);
    SDL_Quit();

    return 0;
}
}
