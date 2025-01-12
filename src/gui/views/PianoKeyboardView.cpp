#include <gui/views/PianoKeyboardView.h>

#include <gui/style/Style.h>

#include <graphics/GraphicsFactory.h>
#include <graphics/Widget.h>

namespace gui::views {

static graphics::Widget createWhiteTouch(
    graphics::GraphicsFactory& graphics_factory,
    int height)
{
    return graphics_factory
        .createWidget("white_touch")
        .addFill(graphics::core::Color::fromHexa("#ffffff"))
        .addSize(0, height)
        .addBorder(1, graphics::core::Color::fromHexa("#000000"));
}

static graphics::Widget createBlackTouch(
    graphics::GraphicsFactory& graphics_factory,
    int height,
    int width)
{
    return graphics_factory
        .createWidget("black_touch")
        .addSize(width, height)
        .addFill(graphics::core::Color::fromHexa("#000000"))
        .addZPosition(1);
}

PianoKeyboardView::PianoKeyboardView(
    graphics::GraphicsFactory& graphics_factory,
    const graphics::Widget& view_widget,
    unsigned octave)
    : m_octave(octave)
{
    const auto view_entity = view_widget.entity;
    const auto view_height = view_widget.height();

    const auto white_note_height = view_height / 7;
    const auto black_note_height = view_height / 12;
    const auto black_note_width = view_widget.width() * 0.7;

    auto b_note
        = createWhiteTouch(graphics_factory, white_note_height)
              .anchorLeft(view_entity, ecs::Left)
              .anchorRight(view_entity, ecs::Right)
              .anchorTop(view_entity, ecs::Top);

    auto a_note
        = createWhiteTouch(graphics_factory, white_note_height)
              .anchorLeft(view_entity, ecs::Left)
              .anchorRight(view_entity, ecs::Right)
              .anchorTop(b_note.entity, ecs::Bottom);

    auto g_note
        = createWhiteTouch(graphics_factory, white_note_height)
              .anchorLeft(view_entity, ecs::Left)
              .anchorRight(view_entity, ecs::Right)
              .anchorTop(a_note.entity, ecs::Bottom);

    auto f_note
        = createWhiteTouch(graphics_factory, white_note_height)
              .anchorLeft(view_entity, ecs::Left)
              .anchorRight(view_entity, ecs::Right)
              .anchorTop(g_note.entity, ecs::Bottom);

    auto e_note
        = createWhiteTouch(graphics_factory, white_note_height)
              .anchorLeft(view_entity, ecs::Left)
              .anchorRight(view_entity, ecs::Right)
              .anchorTop(f_note.entity, ecs::Bottom);

    auto d_note
        = createWhiteTouch(graphics_factory, white_note_height)
              .anchorLeft(view_entity, ecs::Left)
              .anchorRight(view_entity, ecs::Right)
              .anchorTop(e_note.entity, ecs::Bottom);

    auto c_note
        = createWhiteTouch(graphics_factory, white_note_height)
              .anchorLeft(view_entity, ecs::Left)
              .anchorRight(view_entity, ecs::Right)
              .anchorTop(d_note.entity, ecs::Bottom);

    auto c_label
        = graphics_factory
              .createLabel("C" + std::to_string(octave),
                  Style::defaultTextSize(),
                  Style::fontPath(),
                  graphics::core::Color::fromName("black"))
              .anchorTop(c_note.entity, ecs::Top, 7)
              .anchorLeft(c_note.entity, ecs::Left, 3);

    auto c_sharp_note
        = createBlackTouch(graphics_factory, black_note_height, black_note_width)
              .anchorTop(c_note.entity, ecs::Top, (-white_note_height) / 4)
              .anchorLeft(view_entity, ecs::Left);

    auto d_sharp_note
        = createBlackTouch(graphics_factory, black_note_height, black_note_width)
              .anchorTop(d_note.entity, ecs::Top, (-white_note_height) / 4)
              .anchorLeft(view_entity, ecs::Left);

    auto f_sharp_note
        = createBlackTouch(graphics_factory, black_note_height, black_note_width)
              .anchorTop(f_note.entity, ecs::Top, (-white_note_height) / 4)
              .anchorLeft(view_entity, ecs::Left);

    auto g_sharp_note
        = createBlackTouch(graphics_factory, black_note_height, black_note_width)
              .anchorTop(g_note.entity, ecs::Top, (-white_note_height) / 4)
              .anchorLeft(view_entity, ecs::Left);

    auto a_sharp_note
        = createBlackTouch(graphics_factory, black_note_height, black_note_width)
              .anchorTop(a_note.entity, ecs::Top, (-white_note_height) / 4)
              .anchorLeft(view_entity, ecs::Left);
}

}
