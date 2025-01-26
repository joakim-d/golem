#include <gui/views/ConfigurationView.h>

#include <gui/ModalDialog.h>
#include <gui/style/Style.h>
#include <gui/views/instruments/PulseConfigurationView.h>

#include <graphics/GraphicsFactory.h>
#include <model/Song.h>

namespace gui::views {

ConfigurationView::ConfigurationView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget,
    std::shared_ptr<model::Song>
        song_model)
{
    const auto view_widget_z = view_widget.z();
    auto widget
        = graphics_factory
              .createWidget("instruments_header_cell")
              .anchorTop(view_widget.entity, ecs::Top)
              .anchorLeft(view_widget.entity, ecs::Left)
              .anchorRight(view_widget.entity, ecs::Right)
              .addSize(0, 32)
              .addBorder(1, graphics::core::Color::fromHexa("#000000"))
              .addFill(graphics::core::Color::fromHexa("#111111"))
              .addZPosition(view_widget_z);

    graphics_factory
        .createLabel(
            "Instruments",
            Style::defaultTextSize(),
            Style::fontPath(),
            Style::defaultColor())
        .anchorLeft(widget.entity, ecs::Left, 5)
        .anchorTop(widget.entity, ecs::Top)
        .addZPosition(view_widget_z + 1);

    ecs::Entity previous_entity = widget.entity;
    size_t index = 0;
    song_model->visitInstruments(
        [this, &graphics_factory, view_widget, &previous_entity, &index, song_model](model::Instrument& instrument) {
            auto cell
                = graphics_factory.createWidget("instrument_cell")
                      .anchorTop(previous_entity, ecs::Bottom)
                      .anchorLeft(view_widget.entity, ecs::Left)
                      .anchorRight(view_widget.entity, ecs::Right)
                      .addSize(0, 32)
                      .addZPosition(view_widget.z())
                      .addFill(graphics::core::Color::fromHexa(index % 2 == 0 ? "#222222" : "#111111"));
            previous_entity = cell.entity;

            auto label
                = graphics_factory
                      .createLabel(
                          std::to_string(index + 1),
                          Style::defaultTextSize(),
                          Style::fontPath(),
                          Style::defaultColor())
                      .anchorTop(cell.entity, ecs::Top)
                      .anchorLeft(cell.entity, ecs::Left, 5)
                      .addZPosition(view_widget.z() + 1);

            graphics_factory.createImage(Style::imagePath(Style::Image::Pulse))
                .anchorTop(cell.entity, ecs::Top)
                .anchorRight(cell.entity, ecs::Right)
                .addZPosition(view_widget.z() + 1)
                .onPressed(ecs::Button::Left, [this, index, &graphics_factory, song_model](ecs::Button, int, int) {
                    auto pulse_configuration_widget
                        = graphics_factory.createWidget("pulse_configuration");

                    gui::ModalDialog<PulseConfigurationView>::spawn(
                        graphics_factory,
                        graphics_factory,
                        pulse_configuration_widget,
                        index,
                        song_model);
                });

            index++;
        });
}

}
