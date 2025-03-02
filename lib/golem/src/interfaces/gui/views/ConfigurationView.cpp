#include <interfaces/gui/views/ConfigurationView.h>

#include <interfaces/gui/ModalDialog.h>
#include <interfaces/gui/style/Style.h>
#include <interfaces/gui/views/instruments/PulseConfigurationView.h>

#include <golem/domain/Song.h>
#include <interfaces/graphics/GraphicsFactory.h>

namespace gui::views
{

class InstrumentsChangedHandler : public domain::events::IDomainEventHandler<
                                      domain::events::InstrumentsChanged>
{
public:
    InstrumentsChangedHandler(ConfigurationView& view)
        : m_view(view)
    {
    }

    void handleEvent(domain::events::InstrumentsChanged event) override
    {
        m_view.buildView();
    }

private:
    ConfigurationView& m_view;
};

ConfigurationView::ConfigurationView(
    graphics::GraphicsFactory& graphics_factory,
    graphics::Widget& view_widget,
    use_cases::ProjectUseCases& use_cases,
    domain::events::ProjectDomainEventProcessor& events_processor)
    : m_instrument_changed_handler(
        std::make_unique<InstrumentsChangedHandler>(*this))
    , m_graphics_factory(graphics_factory)
    , m_use_cases(use_cases)
    , m_view_widget(view_widget)
    , m_instrument_cell_widget(
          graphics_factory.createWidget("instruments_header_cell")
              .anchorTop(view_widget.entity, ecs::Top)
              .anchorLeft(view_widget.entity, ecs::Left)
              .anchorRight(view_widget.entity, ecs::Right)
              .addSize(0, 32)
              .addBorder(1, graphics::core::Color::fromHexa("#000000"))
              .addFill(graphics::core::Color::fromHexa("#111111"))
              .addZPosition(view_widget.z()))
{
    graphics_factory
        .createLabel(
            "Instruments", Style::defaultTextSize(), Style::fontPath(),
            Style::defaultColor())
        .anchorLeft(m_instrument_cell_widget.entity, ecs::Left, 5)
        .anchorTop(m_instrument_cell_widget.entity, ecs::Top)
        .addZPosition(view_widget.z() + 1);

    events_processor.registerEventHandler(m_instrument_changed_handler.get());

    buildView();
}

void ConfigurationView::buildView()
{
    const auto& instruments = m_use_cases.get_instruments();

    for (auto& widget : m_widgets)
    {
        widget.die();
    }
    m_widgets.clear();

    auto previous_entity = m_instrument_cell_widget.entity;

    size_t index = 0;
    for (const auto& instrument : instruments)
    {
        auto cell = m_graphics_factory.createWidget("instrument_cell")
                        .anchorTop(previous_entity, ecs::Bottom)
                        .anchorLeft(m_view_widget.entity, ecs::Left)
                        .anchorRight(m_view_widget.entity, ecs::Right)
                        .addSize(0, 32)
                        .addZPosition(m_view_widget.z())
                        .addFill(graphics::core::Color::fromHexa(
                            index % 2 == 0 ? "#222222" : "#111111"));
        m_widgets.push_back(cell);

        previous_entity = cell.entity;

        auto label
            = m_graphics_factory
                  .createLabel(
                      std::to_string(index + 1), Style::defaultTextSize(),
                      Style::fontPath(), Style::defaultColor())
                  .anchorTop(cell.entity, ecs::Top)
                  .anchorLeft(cell.entity, ecs::Left, 5)
                  .addZPosition(m_view_widget.z() + 1);

        m_widgets.push_back(label);

        auto pulse_image
            = m_graphics_factory
                  .createImage(Style::imagePath(Style::Image::Pulse))
                  .anchorTop(cell.entity, ecs::Top)
                  .anchorRight(cell.entity, ecs::Right)
                  .addZPosition(m_view_widget.z() + 1)
                  .onPressed(
                      ecs::Button::Left,
                      [this, index](ecs::Button, int, int)
                      {
                          auto pulse_configuration_widget
                              = m_graphics_factory.createWidget(
                                  "pulse_configuration");

                          gui::ModalDialog<PulseConfigurationView>::spawn(
                              m_graphics_factory, m_graphics_factory,
                              pulse_configuration_widget, index, nullptr);
                      });

        m_widgets.push_back(pulse_image);
        index++;
    }
}

}
