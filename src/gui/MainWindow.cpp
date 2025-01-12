#include <gui/MainWindow.h>

#include <gui/MenuBar.h>
#include <gui/style/Style.h>
#include <gui/views/SequencerView.h>
#include <gui/views/SongView.h>

#include <graphics/FontManager.h>
#include <graphics/GraphicsFactory.h>
#include <graphics/LabelFactory.h>
#include <graphics/Widget.h>

#include <ecs/EntityManager.h>
#include <ecs/EntityMemoryPool.h>
#include <ecs/SystemFactory.h>

#include <ecs/components/AnchorComponent.h>
#include <ecs/components/BorderComponent.h>
#include <ecs/components/FillComponent.h>
#include <ecs/components/PositionComponent.h>
#include <ecs/components/SizeComponent.h>

#include <ecs/systems/AnchoringSystem.h>
#include <ecs/systems/RendererSystem.h>
#include <ecs/systems/UpdaterSystem.h>
#include <ecs/systems/UserInputSystem.h>

#include <SDL2/SDL.h>

#include <iostream>

namespace gui {

std::optional<MainWindow> MainWindow::create(
    int width,
    int height,
    const char* title)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

    if (window == nullptr) {
        std::cerr << "Failed to open window" << std::endl;
        return std::nullopt;
    }

    MainWindow main_window;
    main_window.m_window = window;

    return main_window;
}

int MainWindow::execute()
{
    ecs::EntityMemoryPool pool { 1024 * 1024 };

    ecs::EntityManager manager { pool };

    auto renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

    graphics::GraphicsFactory graphics_factory { renderer, manager };

    // Create window widget
    auto window_widget
        = graphics_factory
              .createWidget("window")
              .addSize(800, 0);

    const auto window_entity = window_widget.entity;

    auto menu_bar_widget
        = graphics_factory.createWidget("menu_bar")
              .addSize(0, 32)
              .anchorLeft(window_entity, ecs::Left)
              .anchorRight(window_entity, ecs::Right)
              .anchorTop(window_entity, ecs::Top)
              .addFill(graphics::core::Color::fromHexa("#333333"));

    gui::MenuBar menu_bar(graphics_factory, menu_bar_widget.entity);

    auto song_view_widget
        = graphics_factory.createWidget("song_view")
              .addSize(0, 48 * 4 + 16 + 4) // 48 pixels per track + 16 pixels for header + 4 pixels for spacing
              .anchorLeft(window_entity, ecs::Left)
              .anchorRight(window_entity, ecs::Right)
              .anchorTop(menu_bar_widget.entity, ecs::Bottom);

    gui::views::SongView { graphics_factory, song_view_widget.entity };

    auto sequencer_widget
        = graphics_factory.createWidget("sequencer_view")
              .addSize(window_widget.width(), 0)
              .anchorTop(song_view_widget.entity, ecs::Bottom)
              //.anchorLeft(window_entity, ecs::Left)
              //.anchorRight(window_entity, ecs::Right)
              .anchorBottom(window_entity, ecs::Bottom);

    gui::views::SequencerView sequencer_view {
        graphics_factory,
        sequencer_widget
    };

    ecs::SystemFactory factory { pool, manager };

    int window_width;
    int window_height;

    SDL_GetWindowSize(m_window, &window_width, &window_height);

    auto user_input_system = factory.createSystem<ecs::UserInputSystem>(
        window_width,
        window_height);

    auto updater_system = factory.createSystem<ecs::Updater>();

    auto anchoring_system = factory.createSystem<ecs::AnchoringSystem>();

    auto renderer_system = factory.createSystem<ecs::Renderer>(
        renderer,
        graphics_factory.textureManager());

    while (!SDL_QuitRequested()) {
        user_input_system.update();
        updater_system.update();
        anchoring_system.update();
        renderer_system.update();

        SDL_Delay(1000 / 60);
    }

    SDL_DestroyWindow(m_window);
    SDL_Quit();

    return 0;
}
}
