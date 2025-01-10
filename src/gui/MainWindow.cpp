#include <gui/MainWindow.h>

#include <gui/MenuBar.h>
#include <gui/views/SongView.h>
#include <gui/widgets/Widget.h>

#include <graphics/FontManager.h>
#include <graphics/LabelFactory.h>

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

    auto screen_entity = manager.addEntity("window");

    pool.addComponent(
        *screen_entity,
        ecs::Position { 0, 0 });

    pool.addComponent(
        *screen_entity,
        ecs::Size {
            0,
            0 });

    pool.addComponent(
        *screen_entity,
        ecs::Fill {
            SDL_Color { 0, 0, 0, 255 } });

    auto menu_bar_widget
        = gui::widgets::Widget::
              createEntity("menu_bar", manager)
                  .addSize(0, 32)
                  .anchorLeft(*screen_entity, ecs::Left)
                  .anchorRight(*screen_entity, ecs::Right)
                  .anchorTop(*screen_entity, ecs::Top)
                  .addFill(graphics::core::Color::fromHexa("#333333"));

    gui::MenuBar menu_bar(manager, menu_bar_widget.entity);

    auto song_view_widget
        = gui::widgets::Widget::
              createEntity("song_view", manager)
                  .addSize(0, 16)
                  .anchorLeft(*screen_entity, ecs::Left)
                  .anchorRight(*screen_entity, ecs::Right)
                  .anchorTop(menu_bar_widget.entity, ecs::Bottom);

    gui::views::SongView { manager, song_view_widget.entity };

    ecs::SystemFactory factory { pool, manager };

    int window_width;
    int window_height;
    SDL_GetWindowSize(m_window, &window_width, &window_height);

    auto renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED);

    graphics::FontManager font_manager;
    graphics::TextureManager texture_manager { renderer };

    graphics::LabelFactory label_factory { font_manager, texture_manager, manager };

    auto label_entity = label_factory.createLabel(
        "TESSSSSSSSSSSSSSSSSSSSSST",
        "/home/joachim/.local/share/fonts/JetBrainsMono-Regular.ttf",
        48,
        graphics::core::Color::fromHexa("#0000ff"));

    auto user_input_system = factory.createSystem<ecs::UserInputSystem>(
        window_width,
        window_height);

    auto updater_system = factory.createSystem<ecs::Updater>();
    auto anchoring_system = factory.createSystem<ecs::AnchoringSystem>();
    auto renderer_system = factory.createSystem<ecs::Renderer>(renderer, texture_manager);

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
