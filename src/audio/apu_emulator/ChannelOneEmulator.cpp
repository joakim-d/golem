#include <audio/apu_emulator/ChannelOneEmulator.h>

#include <audio/apu_emulator/AudioRegisters.h>

#include <cmath>
#include <cstring>

#include <optional>

ChannelOneEmulator::ChannelOneEmulator(std::shared_ptr<AudioRegisters> audio_registers)
    : m_audio_registers(std::move(audio_registers))
    , m_waveform_ticks(0)
    , m_volume_ticks(0)
    , m_length_timer_ticks(0)
    , m_waveform_index(0)
    , m_volume(15)
    , m_length_timer(0)
    , m_on(false)
{
}

void ChannelOneEmulator::trigger()
{
    m_volume = m_audio_registers->getChannel1InitialVolume();
    m_length_timer = 64 - m_audio_registers->getChannel1InitialLengthTimer();
    m_on = true;
}

void ChannelOneEmulator::getSamples(std::vector<float>& samples)
{
    if (!m_on) {
        std::fill(samples.begin(), samples.end(), 0.f);
        return;
    }
    constexpr auto CPU_FREQUENCY = 4 * 1024 * 1024;

    constexpr auto TARGET_FREQUENCY = 44100;
    constexpr auto TICK_INCREMENT = CPU_FREQUENCY / TARGET_FREQUENCY;

    float* output = samples.data();

    const bool volume_increase = m_audio_registers->getChannel1EnvelopeDirection();
    const int envelope_sweep_pace = m_audio_registers->getChannel1EnvelopeSweepPace();
    const int sweep_pace = m_audio_registers->getChannel1SweepPace();

    std::optional<size_t> volume_pace_ticks_target;
    std::optional<size_t> sweep_ticks_target;
    const auto length_timer_ticks = CPU_FREQUENCY / 256;

    const bool is_length_timer_enabled = m_audio_registers->getChannel1LengthEnable();

    if (envelope_sweep_pace) {
        volume_pace_ticks_target = (CPU_FREQUENCY * envelope_sweep_pace) / (64);
    }

    if (sweep_pace) {
        sweep_ticks_target = (CPU_FREQUENCY * sweep_pace) / (128);
    }

    for (size_t i = 0; i < samples.size(); ++i) {
        auto rawFrequency = m_audio_registers->getChannel1Period();
        auto frequency = 131072 / (2048 - rawFrequency);
        m_waveform_ticks += TICK_INCREMENT;
        const unsigned waveform_index_increment
            = m_waveform_ticks / (CPU_FREQUENCY / (frequency * 8));
        m_waveform_ticks %= (CPU_FREQUENCY / (frequency * 8));
        m_waveform_index = (m_waveform_index + waveform_index_increment) & 7;

        if (volume_pace_ticks_target.has_value()) {
            m_volume_ticks += TICK_INCREMENT;
            const unsigned volume_ticks_increment = m_volume_ticks / *volume_pace_ticks_target;
            m_volume_ticks %= *volume_pace_ticks_target;
            if (volume_increase) {
                m_volume
                    = m_volume + volume_ticks_increment > 15
                    ? 15
                    : m_volume + volume_ticks_increment;
            } else {
                m_volume
                    = volume_ticks_increment > m_volume
                    ? 0
                    : m_volume - volume_ticks_increment;
            }
        }
        if (sweep_ticks_target.has_value()) {
            m_sweep_ticks += TICK_INCREMENT;
            const unsigned sweep_ticks_increment
                = m_sweep_ticks / *sweep_ticks_target;
            m_sweep_ticks %= *sweep_ticks_target;
            const bool increase = m_audio_registers->getChannel1SweepDirection();
            const int steps = m_audio_registers->getChannel1SweepSteps();
            for (unsigned j = 0; j < sweep_ticks_increment; ++j) {
                unsigned new_period = m_audio_registers->getChannel1Period();
                new_period = increase
                    ? new_period + (new_period / pow(2, steps))
                    : new_period - (new_period / pow(2, steps));

                if (new_period > 0x7FF) {
                    m_on = false;
                } else {
                    m_audio_registers->setChannel1Period(new_period);
                    frequency = 131072 / (2048 - new_period);
                    m_sweep_ticks = 0;
                }
            }
        }
        if (is_length_timer_enabled) {
            m_length_timer_ticks += TICK_INCREMENT;
            const unsigned length_timer_increment
                = m_length_timer_ticks / length_timer_ticks;
            m_length_timer_ticks %= length_timer_ticks;
            m_length_timer
                = length_timer_increment > m_length_timer
                ? 0
                : m_length_timer - length_timer_increment;

            if (m_length_timer == 0) {
                m_on = false;
            }
        }
        float volume = m_volume * (1.f / 16.f);
        float current_value;
        if (m_on) {
            current_value
                = m_audio_registers->getChannel1DutyCycleBitmask() & (1 << m_waveform_index)
                ? volume
                : -volume;
        } else {
            current_value = 0.f;
        }

        *output = current_value;
        output++;
    }
}
