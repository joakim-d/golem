#pragma once

#include <golem/domain/events/DomainEvent.h>
#include <golem/domain/events/InstrumentsChanged.h>
#include <golem/domain/events/PlaybackInfoChanged.h>

namespace domain::events
{

using ProjectDomainEventProcessor
    = DomainEventProcessor<PlaybackInfoChanged, InstrumentsChanged>;

}
