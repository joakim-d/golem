#include <core/NotePlayer.h>

#include <model/Song.h>
#include <model/instruments/Instrument.h>

#include <core/IAudioProcessingUnit.h>

#include <core/Common.h>

namespace core {

NotePlayer::NotePlayer(
    std::shared_ptr<model::Song> song_model,
    std::shared_ptr<IAudioProcessingUnit> audio_processing_unit)
    : m_song_model(std::move(song_model))
    , m_audio_processing_unit(std::move(audio_processing_unit))
{
}

void NotePlayer::play(
    Channel channel,
    size_t instrument_index,
    std::optional<model::Note> note)
{
    auto& instrument = m_song_model->getInstrument(instrument_index);
    switch (channel) {
    case Channel::Channel1:
        setChannel1Instrument(instrument.pulse(), note);
        break;
    case Channel::Channel2:
        setChannel2Instrument(instrument.pulse(), note);
        break;
    case Channel::Channel3:
        setChannel3Instrument(instrument.wave(), note);
        break;
    default:
        break;
    }
}

void NotePlayer::setChannel1Instrument(
    const model::PulseInstrument& pulse,
    std::optional<model::Note> note)
{
    const uint8_t register_10
        = (pulse.sweepPace() << 4)
        | (pulse.sweepIncrease() << 3)
        | (pulse.sweepSteps());

    const uint8_t register_11
        = ((unsigned)pulse.dutyCycle() << 6)
        | (pulse.initialLengthTimer());

    const uint8_t register_12
        = (pulse.envelopeInitialVolume() << 4)
        | (pulse.envelopeIncrease() << 3)
        | (pulse.envelopePace());

    m_audio_processing_unit->updateRegister(
        Register::NR10, register_10);

    m_audio_processing_unit->updateRegister(
        Register::NR11, register_11);

    m_audio_processing_unit->updateRegister(
        Register::NR12, register_12);

    if (note.has_value()) {
        const auto period = noteToPulsePeriod(*note);

        m_audio_processing_unit->updateRegister(
            Register::NR13, period);

        const auto register_14
            = 0x80
            | (pulse.lengthEnable() << 6)
            | (period >> 8);

        m_audio_processing_unit->updateRegister(
            Register::NR14,
            register_14);
    } else {
        const auto register_14
            = (pulse.lengthEnable() << 6);

        m_audio_processing_unit->updateRegister(
            Register::NR14,
            register_14,
            1 << 6);
    }
}

void NotePlayer::setChannel2Instrument(
    const model::PulseInstrument& pulse,
    std::optional<model::Note> note)
{
    const uint8_t register_21
        = ((unsigned)pulse.dutyCycle() << 6)
        | (pulse.initialLengthTimer());

    const uint8_t register_22
        = (pulse.envelopeInitialVolume() << 4)
        | (pulse.envelopeIncrease() << 3)
        | (pulse.envelopePace());

    m_audio_processing_unit->updateRegister(
        Register::NR21, register_21);

    m_audio_processing_unit->updateRegister(
        Register::NR22, register_22);

    if (note.has_value()) {
        const auto period = noteToPulsePeriod(*note);

        m_audio_processing_unit->updateRegister(
            Register::NR23, period);

        const auto register_24
            = 0x80
            | (pulse.lengthEnable() << 6)
            | (period >> 8);

        m_audio_processing_unit->updateRegister(
            Register::NR24,
            register_24);
    } else {
        const auto register_24
            = (pulse.lengthEnable() << 6);

        m_audio_processing_unit->updateRegister(
            Register::NR24,
            register_24,
            1 << 6);
    }
}

void NotePlayer::setChannel3Instrument(
    const model::WaveInstrument& wave,
    std::optional<model::Note> note)
{
    // ugly comparison, should we try to compare by instrument index ?
    if (wave != m_wave) {
        m_wave = wave;
        // Disable DAC
        m_audio_processing_unit->updateRegister(
            Register::NR30, 0);

        m_audio_processing_unit->updateRegister(
            Register::NR31, m_wave.lengthTimer());

        m_audio_processing_unit->updateRegister(
            Register::NR32, m_wave.volume() << 5);

        m_audio_processing_unit->updateWaveRam(
            m_wave.getRawSamples());

        // Enable DAC back
        m_audio_processing_unit->updateRegister(
            Register::NR30, 0x80);
    }

    if (note) {
        const auto period = noteToWavePeriod(*note);

        m_audio_processing_unit->updateRegister(
            Register::NR33, period & 0xFF);

        const uint8_t nr34
            = 0x80
            | (wave.lengthEnable() << 6)
            | (period >> 8);

        m_audio_processing_unit->updateRegister(
            Register::NR34, nr34);
    }
}

}
