#pragma once

#include <atomic>
#include <memory>
#include <thread>

namespace model {
class Chain;
class Phrase;
class Song;
}

class IAudioProcessingUnit;

namespace core {

class IPlayingProgressionTracker;
class NotePlayer;

class SongPlayer {
public:
    SongPlayer(
        std::shared_ptr<model::Song> song,
        std::shared_ptr<IAudioProcessingUnit> audio_processing_unit,
        std::shared_ptr<IPlayingProgressionTracker> progression_tracker,
        std::shared_ptr<core::NotePlayer>
            instrument_affector);

    ~SongPlayer();

    void playSong();
    void stop();

    bool isStopped() const;

private:
    void playSong(const model::Song& song);

    std::thread m_thread;
    std::shared_ptr<model::Song> m_song;
    std::shared_ptr<IAudioProcessingUnit> m_audio_processing_unit;
    std::shared_ptr<IPlayingProgressionTracker> m_playing_progression_tracker;
    std::shared_ptr<core::NotePlayer> m_note_player;

    std::atomic_bool m_stop;
};

}
