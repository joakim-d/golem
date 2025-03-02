#pragma once

#include <tuple>
#include <vector>

namespace domain::events
{

template <typename Event>
class IDomainEventHandler
{
public:
    virtual ~IDomainEventHandler() = default;
    virtual void handleEvent(Event event) = 0;
};

template <typename Event>
class DomainEventPublisher
{
public:
    void registerHandler(IDomainEventHandler<Event>* event_handler)
    {
        m_event_handlers.push_back(event_handler);
    }

    void publishEvent(Event event)
    {
        for (auto& handler : m_event_handlers)
        {
            handler->handleEvent(event);
        }
    }

private:
    std::vector<IDomainEventHandler<Event>*> m_event_handlers;
};

template <typename... Events>
class DomainEventProcessor
{
public:
    template <typename Event>
    void publishEvent(Event event)
    {
        std::get<DomainEventPublisher<Event>>(m_event_publishers)
            .publishEvent(event);
    }

    template <typename Event>
    void registerEventHandler(IDomainEventHandler<Event>* event_handler)
    {
        std::get<DomainEventPublisher<Event>>(m_event_publishers)
            .registerHandler(event_handler);
    }

private:
    std::tuple<DomainEventPublisher<Events>...> m_event_publishers;
};

}
