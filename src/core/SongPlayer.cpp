#include <core/SongPlayer.h>

#include <core/IAudioProcessingUnit.h>
#include <core/IPlayingProgressionTracker.h>
#include <core/NotePlayer.h>

#include <model/Note.h>
#include <model/Song.h>

namespace core {

SongPlayer::SongPlayer(
    std::shared_ptr<model::Song> song,
    std::shared_ptr<IAudioProcessingUnit> audio_processing_unit,
    std::shared_ptr<IPlayingProgressionTracker> progression_tracker,
    std::shared_ptr<core::NotePlayer> note_player)
    : m_song(std::move(song))
    , m_audio_processing_unit(std::move(audio_processing_unit))
    , m_playing_progression_tracker(std::move(progression_tracker))
    , m_note_player(std::move(note_player))
    , m_stop(false)
{
}

SongPlayer::~SongPlayer()
{
    stop();
}

void SongPlayer::playSong()
{
    stop();
    m_stop = false;
    m_thread = std::thread { [this]() {
        playSong(*m_song);
        m_stop = true; } };
}

void SongPlayer::playSong(const model::Song& song)
{
    struct PlayStatus {
        size_t currentPhrase = 0;
        size_t currentNote = 0;
        size_t currentTick = 0;
        size_t durationLeft = 0;
        bool stopped = false;
    };

    PlayStatus statuses[model::Song::TRACK_COUNT];
    PlayStatus globalStatus;
    // If no chain index at the beginning we consider the track to be already over
    for (size_t track_index = 0; track_index < model::Song::TRACK_COUNT; ++track_index) {
        statuses[track_index].stopped = !song.getTrack(track_index).phraseIndex(0).has_value();
    }

    while (!m_stop) {
        for (size_t track_index = 0; track_index < model::Song::TRACK_COUNT; ++track_index) {
            auto& status = statuses[track_index];
            if (status.stopped)
                continue;

            status.currentTick++;
            if (status.currentTick != m_song->ticksPerNote()) {
                continue;
            }
            status.currentTick = 0;

            const auto& track = song.getTrack(track_index);
            const auto& phrase_index = track.phraseIndex(status.currentPhrase);

            const auto& phrase = song.getPhrase(*phrase_index);

            const auto& note = phrase.note(status.currentNote);

            if (note.has_value()) {
                m_note_player->play(
                    static_cast<Channel>(
                        (unsigned)(Channel::Channel1) + track_index),
                    note->instrumentIndex(),
                    note->frequency());
                status.durationLeft = note->duration();
            } else {
                if (status.durationLeft > 0) {
                    status.durationLeft--;
                    if (status.durationLeft == 0) {
                        m_note_player->stop(
                            static_cast<Channel>(
                                unsigned(Channel::Channel1)
                                + track_index));
                    }
                }
            }

            status.currentNote++;
            if (status.currentNote != model::Phrase::NOTE_COUNT) {
                continue;
            }

            status.currentNote = 0;
            status.currentPhrase++;
            if (track.phraseIndex(status.currentPhrase).has_value()) {
                continue;
            }
            status.stopped = true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds { 1000 / 60 });
        if (!m_stop) {
            bool quit = true;
            for (size_t track_index = 0; track_index < model::Song::TRACK_COUNT; ++track_index) {
                if (!statuses[track_index].stopped) {
                    quit = false;
                }
            }
            m_stop = quit;
        }
        globalStatus.currentTick++;
        if (globalStatus.currentTick != m_song->ticksPerNote()) {
            m_playing_progression_tracker->onProgressMade(
                globalStatus.currentTick,
                globalStatus.currentNote,
                globalStatus.currentPhrase);
            continue;
        }
        globalStatus.currentTick = 0;
        globalStatus.currentNote++;
        if (globalStatus.currentNote != model::Phrase::NOTE_COUNT) {
            m_playing_progression_tracker->onProgressMade(
                globalStatus.currentTick,
                globalStatus.currentNote,
                globalStatus.currentPhrase);
            continue;
        }
        globalStatus.currentPhrase++;
        globalStatus.currentNote = 0;
        m_playing_progression_tracker->onProgressMade(
            globalStatus.currentTick,
            globalStatus.currentNote,
            globalStatus.currentPhrase);
    }

    m_note_player->stop(Channel::Channel1);
    m_note_player->stop(Channel::Channel2);
    m_note_player->stop(Channel::Channel3);
    m_note_player->stop(Channel::Channel4);
}

void SongPlayer::stop()
{
    m_stop = true;
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

bool SongPlayer::isStopped() const
{
    return !m_thread.joinable();
}

}
