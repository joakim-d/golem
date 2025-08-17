#include <golem/core/SongPlayer.h>

#include <golem/core/IAudioProcessingUnit.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

#include <golem/core/NotePlayer.h>

#include <golem/domain/Note.h>
#include <golem/domain/Song.h>

namespace core
{

SongPlayer::SongPlayer(
    domain::IProjectRepository& project_repository,
    domain::events::ProjectDomainEventProcessor& event_processor,
    std::shared_ptr<IAudioProcessingUnit> audio_processing_unit,
    core::NotePlayer& note_player)
    : m_project_repository(project_repository)
    , m_event_processor(event_processor)
    , m_audio_processing_unit(std::move(audio_processing_unit))
    , m_note_player(note_player)
    , m_tick_handle(m_audio_processing_unit->tick.register_callback(
          std::bind(&SongPlayer::tick, this)))
{
    m_global_status.stopped = true;
}

SongPlayer::~SongPlayer()
{
}

void SongPlayer::playSong(size_t index)
{
    for (size_t i = 0; i < 4; ++i)
    {
        m_global_status.paused[i] = false;
    }
    m_song_index = index;
    if (m_global_status.stopped)
    {
        m_global_status.stopped = false;
        m_global_status = domain::PlaybackInfo {};
    }
}

void SongPlayer::tick()
{
    return;
    /*
    if (m_global_status.paused || m_global_status.stopped)
    {
        return;
    }

    auto song = m_project_repository.getProject().song(m_song_index);

    if (song == nullptr)
    {
        return;
    }

    if (m_global_status.current_tick == 0)
    {
        for (size_t track_index = 0; track_index < domain::Song::TRACK_COUNT;
             ++track_index)
        {
            const auto& track = song->getTrack(track_index);

            m_global_status.current_pattern[track_index]

                const auto& note
                = std::optional<domain::Note> {};

            m_global_status.
                // track.note(m_global_status..current_note);

                if (note.has_value())
            {
                m_note_player.play(
                    note->instrumentIndex(),
                    static_cast<domain::Channel>(track_index),
                    note->frequency());

                m_channel_statuses[track_index].duration_left
                    = note->duration();
            }
            else
            {
                if (m_channel_statuses[track_index].duration_left > 0)
                {
                    m_channel_statuses[track_index].duration_left--;
                    if (m_channel_statuses[track_index].duration_left == 0)
                    {
                        m_note_player.stop(
                            static_cast<domain::Channel>(track_index));
                    }
                }
            }
        }
    }

    m_global_status.current_tick++;
    if (m_global_status.current_tick != song->ticksPerNote())
    {
        m_event_processor.publishEvent(m_global_status);
        return;
    }
    m_global_status.current_tick = 0;
    m_global_status.current_note++;

    if (m_global_status.current_note != song->notesCount())
    {
        m_event_processor.publishEvent(m_global_status);
        return;
    }

    m_note_player.stop(domain::Channel::Channel1);
    m_note_player.stop(domain::Channel::Channel2);
    m_note_player.stop(domain::Channel::Channel3);
    m_note_player.stop(domain::Channel::Channel4);

    m_global_status.stopped = true;
    m_event_processor.publishEvent(m_global_status);
*/
}

void SongPlayer::pause()
{
    // m_global_status.paused = true;
    m_note_player.stop(domain::Channel::Channel1);
    m_note_player.stop(domain::Channel::Channel2);
    m_note_player.stop(domain::Channel::Channel3);
    m_note_player.stop(domain::Channel::Channel4);
}

void SongPlayer::stop()
{
    m_global_status.stopped = true;
    m_note_player.stop(domain::Channel::Channel1);
    m_note_player.stop(domain::Channel::Channel2);
    m_note_player.stop(domain::Channel::Channel3);
    m_note_player.stop(domain::Channel::Channel4);
}

const domain::PlaybackInfo& SongPlayer::getPlaybackInfo() const
{
    return m_global_status;
}

}
