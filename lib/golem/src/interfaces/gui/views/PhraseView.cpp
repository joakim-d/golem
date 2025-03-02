#include <interfaces/gui/views/PhraseView.h>

#include <interfaces/graphics/GraphicsFactory.h>

namespace gui::views
{

namespace
{

    static graphics::core::Color COLORS[12] {
        graphics::core::Color::fromHexa("#222222"),
        graphics::core::Color::fromHexa("#111111"),
        graphics::core::Color::fromHexa("#222222"),
        graphics::core::Color::fromHexa("#111111"),
        graphics::core::Color::fromHexa("#222222"),
        graphics::core::Color::fromHexa("#111111"),
        graphics::core::Color::fromHexa("#222222"),
        graphics::core::Color::fromHexa("#222222"),
        graphics::core::Color::fromHexa("#111111"),
        graphics::core::Color::fromHexa("#222222"),
        graphics::core::Color::fromHexa("#111111"),
        graphics::core::Color::fromHexa("#222222"),
    };

}

PhraseView::PhraseView(
    graphics::GraphicsFactory& graphics_factory,
    const graphics::Widget& view_widget)
{
    const int noteHeight = 13;
    const int noteWidth = 15;

    ecs::Entity top_entity = view_widget.entity;
    ecs::Entity left_entity = view_widget.entity;

    bool first_done_top = false;
    bool first_done_left = false;
    for (size_t sequencer_note = 0; sequencer_note < 16; ++sequencer_note)
    {
        for (size_t keyboard_note = 0; keyboard_note < 12 * 6; ++keyboard_note)
        {
            auto note_widget
                = graphics_factory.createWidget("note")
                      .addSize(noteWidth, noteHeight)
                      .anchorTop(
                          top_entity, first_done_top ? ecs::Bottom : ecs::Top,
                          1)
                      .anchorLeft(
                          left_entity, first_done_left ? ecs::Right : ecs::Left,
                          1)
                      .addFill(COLORS[keyboard_note % 12]);
            first_done_top = true;
            top_entity = note_widget.entity;
        }
        left_entity = top_entity;
        top_entity = view_widget.entity;
        first_done_top = false;
        first_done_left = true;
    }
}

}
