#include <golem/core/NotePlayer.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>
#include <golem/domain/Register.h>
#include <golem/domain/Song.h>
#include <golem/domain/instruments/Instrument.h>

#include <golem/core/IAudioProcessingUnit.h>

#include <core/Common.h>

namespace core
{

NotePlayer::NotePlayer(
    domain::IProjectRepository& project_repository,
    std::shared_ptr<IAudioProcessingUnit> audio_processing_unit)
    : m_project_repository(project_repository)
    , m_audio_processing_unit(std::move(audio_processing_unit))
    , m_handle(m_audio_processing_unit->tick.register_callback(
          std::bind(&NotePlayer::tick, this)))
    , m_note(domain::NoteFrequency::C2)
{
}

void NotePlayer::play(
    size_t instrument_index,
    domain::Channel channel,
    domain::NoteFrequency note)
{
    auto instrument
        = m_project_repository.getProject().instrument(instrument_index);

    if (instrument == nullptr)
    {
        return;
    }

    switch (channel)
    {
    case domain::Channel::Channel1:
        m_tick_counter = 0;
        m_note = note;
        setChannel1Instrument(instrument->pulse(), note);
        break;
    case domain::Channel::Channel2:
        setChannel2Instrument(instrument->pulse(), note);
        break;
    case domain::Channel::Channel3:
        setChannel3Instrument(instrument->wave(), note);
        break;
    default:
        break;
    }
}

void NotePlayer::stop(domain::Channel channel)
{
    switch (channel)
    {
    case domain::Channel::Channel1:
        stopChannel1();
        break;
    case domain::Channel::Channel2:
        stopChannel2();
        break;
    case domain::Channel::Channel3:
        stopChannel3();
        break;
    default:
        break;
    }
}

void NotePlayer::tick()
{
    int arpeggio[] = { 0 };

    const auto period = noteToPulsePeriod(domain::NoteFrequency {
        int(domain::NoteFrequency(m_note))
        + arpeggio
            [m_tick_counter % (sizeof(arpeggio) / sizeof(arpeggio[0]))] });

    m_audio_processing_unit->updateRegister(domain::NR13, period);

    const auto register_14 = 0x80 | (period >> 8);

    m_audio_processing_unit->updateRegister(
        domain::Register::NR14, register_14);

    m_tick_counter++;
}

void NotePlayer::setChannel1Instrument(
    const domain::PulseInstrument& pulse,
    domain::NoteFrequency note)
{
    const uint8_t register_10 = (pulse.sweepPace() << 4)
        | (pulse.sweepIncrease() << 3) | (pulse.sweepSteps());

    const uint8_t register_11
        = ((unsigned)pulse.dutyCycle() << 6) | (pulse.initialLengthTimer());

    const uint8_t register_12 = (pulse.envelopeInitialVolume() << 4)
        | (pulse.envelopeIncrease() << 3) | (pulse.envelopePace());

    m_audio_processing_unit->updateRegister(
        domain::Register::NR10, register_10);

    m_audio_processing_unit->updateRegister(
        domain::Register::NR11, register_11);

    m_audio_processing_unit->updateRegister(
        domain::Register::NR12, register_12);

    const auto period = noteToPulsePeriod(note);

    m_audio_processing_unit->updateRegister(domain::Register::NR13, period);

    const auto register_14 = 0x80 | (period >> 8);

    m_audio_processing_unit->updateRegister(
        domain::Register::NR14, register_14);
}

void NotePlayer::setChannel2Instrument(
    const domain::PulseInstrument& pulse,
    domain::NoteFrequency note)
{
    const uint8_t register_21
        = ((unsigned)pulse.dutyCycle() << 6) | (pulse.initialLengthTimer());

    const uint8_t register_22 = (pulse.envelopeInitialVolume() << 4)
        | (pulse.envelopeIncrease() << 3) | (pulse.envelopePace());

    m_audio_processing_unit->updateRegister(
        domain::Register::NR21, register_21);

    m_audio_processing_unit->updateRegister(
        domain::Register::NR22, register_22);

    const auto period = noteToPulsePeriod(note);

    m_audio_processing_unit->updateRegister(domain::Register::NR23, period);

    const auto register_24 = 0x80 | (period >> 8);

    m_audio_processing_unit->updateRegister(
        domain::Register::NR24, register_24);
}

void NotePlayer::setChannel3Instrument(
    const domain::WaveInstrument& wave,
    domain::NoteFrequency note)
{
    // ugly comparison, should we try to compare by instrument index ?
    if (wave != m_wave)
    {
        m_wave = wave;
        // Disable DAC
        m_audio_processing_unit->updateRegister(domain::Register::NR30, 0);

        m_audio_processing_unit->updateRegister(
            domain::Register::NR31, m_wave.lengthTimer());

        m_audio_processing_unit->updateRegister(
            domain::Register::NR32, m_wave.volume() << 5);

        m_audio_processing_unit->updateWaveRam(m_wave.getRawSamples());

        // Enable DAC back
        m_audio_processing_unit->updateRegister(domain::Register::NR30, 0x80);
    }

    const auto period = noteToWavePeriod(note);

    m_audio_processing_unit->updateRegister(
        domain::Register::NR33, period & 0xFF);

    const uint8_t nr34 = 0x80 | (period >> 8);

    m_audio_processing_unit->updateRegister(domain::Register::NR34, nr34);
}

void NotePlayer::stopChannel1()
{
    const uint8_t register_12
        = 0; // Set initial volume to 0, decrease env dir so the sound is muted
    const uint8_t register_14 = 0x80; // Trigger
    m_audio_processing_unit->updateRegister(
        domain::Register::NR12, register_12);
    m_audio_processing_unit->updateRegister(
        domain::Register::NR14, register_14);
}

void NotePlayer::stopChannel2()
{
    const uint8_t register_22
        = 0; // Set initial volume to 0, decrease env dir so the sound is muted
    const uint8_t register_24 = 0x80; // Trigger
    m_audio_processing_unit->updateRegister(
        domain::Register::NR22, register_22);
    m_audio_processing_unit->updateRegister(
        domain::Register::NR24, register_24);
}

void NotePlayer::stopChannel3()
{
    const uint8_t register_32
        = 0; // Set initial volume to 0, decrease env dir so the sound is muted
    const uint8_t register_34 = 0x80; // Trigger
    m_audio_processing_unit->updateRegister(
        domain::Register::NR32, register_32);
    m_audio_processing_unit->updateRegister(
        domain::Register::NR34, register_34);
}

}
