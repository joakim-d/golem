#pragma once

#include <interfaces/graphics/Widget.h>

#include <golem/utils/observable.h>

#include <golem/use_cases/ProjectUseCases.h>

namespace domain
{
class Song;
}

namespace use_cases
{
class ProjectUseCases;
}

namespace graphics
{
class GraphicsFactory;
class Widget;
}

namespace gui::views
{

class ConfigurationView
{
    friend class InstrumentsChangedHandler;

public:
    utils::observable<size_t> configurePulseClicked;

    ConfigurationView(
        graphics::GraphicsFactory& graphics_factory,
        graphics::Widget& view_widget,
        use_cases::ProjectUseCases& project_use_cases,
        domain::events::ProjectDomainEventProcessor& events_processor);

private:
    void buildView();

    std::unique_ptr<
        domain::events::IDomainEventHandler<domain::events::InstrumentsChanged>>
        m_instrument_changed_handler;

    graphics::GraphicsFactory& m_graphics_factory;
    graphics::Widget m_view_widget;
    use_cases::ProjectUseCases& m_use_cases;
    graphics::Widget m_instrument_cell_widget;

    std::vector<graphics::Widget> m_widgets;
};

}
