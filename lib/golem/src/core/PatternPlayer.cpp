#include <golem/core/IAudioProcessingUnit.h>
#include <golem/core/NotePlayer.h>
#include <golem/core/PatternPlayer.h>
#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Pattern.h>
#include <golem/domain/Project.h>
#include <golem/use_cases/ProjectUseCases.h>

namespace core
{

PatternPlayer::PatternPlayer(
    domain::IProjectRepository& repository,
    std::shared_ptr<IAudioProcessingUnit> audio_processing_unit,
    core::NotePlayer& note_player)
    : m_repository(repository)
    , m_audio_processing_unit(audio_processing_unit)
    , m_note_player(note_player)
    , m_tick_handle(m_audio_processing_unit->tick.register_callback(
          std::bind(&PatternPlayer::tick, this)))
{
}

void PatternPlayer::play(
    size_t ticks_per_note,
    size_t track_index,
    size_t pattern_index)
{
    m_current_state = CurrentState {};
    m_current_state.ticks_per_note = ticks_per_note;
    m_current_state.track_index = track_index;
    m_current_state.pattern_index = pattern_index;
    m_current_state.duration_left = 1;
    m_current_state.stopped = false;
}

void PatternPlayer::tick()
{
    if (m_current_state.stopped)
    {
        return;
    }

    auto pattern
        = m_repository.getProject().pattern(m_current_state.pattern_index);

    if (m_current_state.ticks_left != 0)
    {
        m_current_state.ticks_left--;
        return;
    }

    m_current_state.ticks_left = m_current_state.ticks_per_note;

    if (m_current_state.duration_left != 0)
    {
        m_current_state.duration_left--;
        return;
    }

    const auto& note = pattern->note(m_current_state.note_index);
    if (note.has_value())
    {
        m_note_player.play(
            note->instrumentIndex(),
            static_cast<domain::Channel>(m_current_state.track_index),
            note->frequency());
        m_current_state.duration_left = note->duration() - 1;
        m_current_state.note_index += note->duration();
    }
    else
    {
        m_note_player.stop(
            static_cast<domain::Channel>(m_current_state.track_index));
        m_current_state.duration_left = 0;
        m_current_state.note_index++;
    }

    m_current_state.ticks_left = m_current_state.ticks_per_note;
    if (m_current_state.note_index == pattern->noteCount())
    {
        m_current_state.note_index = 0;
    }
}

void PatternPlayer::stop()
{
    m_note_player.stop(
        static_cast<domain::Channel>(m_current_state.track_index));
    m_current_state.stopped = true;
}

}
