#include <audio/ApuEmulator.h>

#include <audio/apu_emulator/AudioRegisters.h>
#include <core/IAudioOutputBuffer.h>

#include <cmath>
#include <fstream>

static std::ofstream output;

namespace audio {

ApuEmulator::ApuEmulator(
    std::shared_ptr<IAudioOutputBuffer> audio_output)
    : m_audio_output(std::move(audio_output))
    , m_audio_registers(std::make_shared<AudioRegisters>())
    , m_channel_one_emulator(m_audio_registers)
    , m_channel_two_emulator(m_audio_registers)
    , m_channel_three_emulator(m_audio_registers)
{
    m_audio_output->setGetSamplesCallback(
        std::bind(
            &ApuEmulator::getSamplesCallback,
            this,
            std::placeholders::_1,
            std::placeholders::_2));

    output.open("/tmp/apu_emulator.txt");
}

void ApuEmulator::updateRegister(
    Register reg,
    uint8_t value,
    uint8_t mask)
{
    m_audio_registers->setRegisterValue(
        reg,
        value,
        mask);

    if (reg == Register::NR14) {
        if (value & mask & 0x80)
            m_channel_one_emulator.trigger();
    } else if (reg == Register::NR24) {
        if (value & mask & 0x80)
            m_channel_two_emulator.trigger();
    } else if (reg == Register::NR34) {
        if (value & mask & 0x80)
            m_channel_three_emulator.trigger();
    }
}

void ApuEmulator::updateWaveRam(
    const std::array<uint8_t, 16>& wave_ram)
{
    m_audio_registers->setWaveRam(wave_ram);
}

int16_t getValueFromNormalized(float normalized)
{
    return static_cast<int16_t>(normalized * 32767.f);
}

void ApuEmulator::getSamplesCallback(char* data, size_t length)
{
    const size_t sample_count
        = length
        / (m_audio_output->channelCount()
            * IAudioOutputBuffer::byteCountForFormat(
                m_audio_output->format()));

    const auto target_frequency = m_audio_output->frequency();

    m_channel_one_samples.resize(sample_count);
    m_channel_two_samples.resize(sample_count);
    m_channel_three_samples.resize(sample_count);

    m_channel_one_emulator.getSamples(m_channel_one_samples);
    m_channel_two_emulator.getSamples(m_channel_two_samples);

    m_channel_three_emulator.getSamples(
        m_channel_three_samples,
        target_frequency);

    int16_t* left_output = reinterpret_cast<int16_t*>(data);
    int16_t* right_output = left_output + 1;

    for (size_t i = 0; i < sample_count; ++i) {
        int16_t currentValue
            = (getValueFromNormalized(m_channel_one_samples[i])
                  + getValueFromNormalized(m_channel_two_samples[i])
                  + getValueFromNormalized(m_channel_three_samples[i]))
            / 3.f;

        *left_output = currentValue;
        *right_output = currentValue;
        left_output += 2;
        right_output += 2;
    }
}

}
