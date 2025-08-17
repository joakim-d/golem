#pragma once

#include <golem/utils/observable.h>
#include <memory>

namespace domain
{
class IProjectRepository;
}

class IAudioProcessingUnit;

namespace core
{
class NotePlayer;

class PatternPlayer
{
public:
    PatternPlayer(
        domain::IProjectRepository& repository,
        std::shared_ptr<IAudioProcessingUnit> audio_processing_unit,
        core::NotePlayer& note_player);

    void
    play(size_t ticks_per_second, size_t track_index, size_t pattern_index);

    void stop();

private:
    struct CurrentState
    {
        size_t ticks_per_note { 0 };
        size_t track_index { 0 };
        size_t pattern_index { 0 };
        size_t note_index { 0 };
        size_t duration_left { 0 };
        size_t ticks_left { 0 };
        bool stopped { false };
    };
    void tick();
    domain::IProjectRepository& m_repository;
    std::shared_ptr<IAudioProcessingUnit> m_audio_processing_unit;
    core::NotePlayer& m_note_player;
    CurrentState m_current_state;
    utils::observable_handle m_tick_handle;
};

}
