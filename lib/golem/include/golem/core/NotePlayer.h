#pragma once

#include <golem/domain/Channel.h>
#include <golem/domain/Note.h>
#include <golem/domain/instruments/WaveInstrument.h>

#include <golem/utils/observable.h>

#include <memory>

class IAudioProcessingUnit;

namespace domain
{
class Instrument;
class IProjectRepository;
class PulseInstrument;
class Song;
class WaveInstrument;
}

namespace core
{

class NotePlayer
{
public:
    NotePlayer(
        domain::IProjectRepository& project_repository,
        std::shared_ptr<IAudioProcessingUnit> audio_processing_unit);

    void play(
        size_t instrument_index,
        domain::Channel channel,
        domain::NoteFrequency note);

    void stop(domain::Channel channel);

private:
    void tick();
    void setChannel1Instrument(
        const domain::PulseInstrument& pulse,
        domain::NoteFrequency note);

    void setChannel2Instrument(
        const domain::PulseInstrument& pulse,
        domain::NoteFrequency note);

    void setChannel3Instrument(
        const domain::WaveInstrument& wave,
        domain::NoteFrequency note);

    void stopChannel1();
    void stopChannel2();
    void stopChannel3();

    domain::IProjectRepository& m_project_repository;
    size_t m_tick_counter;
    domain::NoteFrequency m_note;
    std::shared_ptr<IAudioProcessingUnit> m_audio_processing_unit;
    domain::WaveInstrument m_wave;
    utils::observable_handle m_handle;
};

}
