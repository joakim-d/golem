#include <interfaces/gui/views/SequencerHeaderView.h>

#include <interfaces/graphics/GraphicsFactory.h>
#include <interfaces/gui/style/Style.h>

namespace gui::views {

constexpr auto PHRASE_COUNT = 16;
constexpr auto HEADER_WIDTH = 256;
constexpr auto HEADER_HEIGHT = 16;

SequencerHeaderView::SequencerHeaderView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& sequencer_view_widget)
    : m_x_offset(0)
{
    ecs::Entity previous_entity = sequencer_view_widget.entity;

    for (int i = 0; i < PHRASE_COUNT; ++i) {
        auto phrase_header
            = graphics_factory.createWidget("phrase_header")
                  .addSize(HEADER_WIDTH, HEADER_HEIGHT)
                  .addFill(graphics::core::Color::fromHexa("#111111"))
                  .anchorTop(sequencer_view_widget.entity, ecs::Top)
                  .anchorLeft(previous_entity, i == 0 ? ecs::Left : ecs::Right)
                  .anchorBottom(sequencer_view_widget.entity, ecs::Bottom);

        if (i == 0) {
            phrase_header.addUpdateCallback([phrase_header, sequencer_view_widget, this]() {
                auto widget = phrase_header;
                widget.anchorLeft(sequencer_view_widget.entity, ecs::Left, -m_x_offset);
            });
        }

        previous_entity = phrase_header.entity;

        graphics_factory.createLabel(
                            std::to_string(i + 1),
                            Style::defaultTextSize(),
                            Style::fontPath(),
                            Style::defaultColor())
            .anchorLeft(phrase_header.entity, ecs::Left)
            .anchorTop(phrase_header.entity, ecs::Top);
    }
}

void SequencerHeaderView::setXOffset(int x_offset)
{
    m_x_offset = x_offset;
}

}
