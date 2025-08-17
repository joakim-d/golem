#pragma once

#include <memory>

#include <golem/domain/PlaybackInfo.h>
#include <golem/domain/events/ProjectDomainEventProcessor.h>
#include <golem/utils/observable.h>

namespace domain
{
class Chain;
class Phrase;
class Song;
class IProjectRepository;
}

class IAudioProcessingUnit;

namespace core
{

class IPlayingProgressionTracker;
class NotePlayer;

class SongPlayer
{
public:
    SongPlayer(
        domain::IProjectRepository& project_repository,
        domain::events::ProjectDomainEventProcessor& event_processor,
        std::shared_ptr<IAudioProcessingUnit> audio_processing_unit,
        core::NotePlayer& note_player);

    ~SongPlayer();

    void playSong(size_t song_index);

    void stop();

    void pause();

    const domain::PlaybackInfo& getPlaybackInfo() const;

private:
    void tick();

    domain::IProjectRepository& m_project_repository;
    domain::events::ProjectDomainEventProcessor& m_event_processor;
    size_t m_song_index;
    std::shared_ptr<IAudioProcessingUnit> m_audio_processing_unit;
    core::NotePlayer& m_note_player;
    utils::observable_handle m_tick_handle;

    struct ChannelPlayStatus
    {
        unsigned duration_left;
    };

    domain::PlaybackInfo m_global_status;
    ChannelPlayStatus m_channel_statuses[4]; // one status per track;
};

}
