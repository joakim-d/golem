#include <golem/audio/apu_emulator/ChannelThreeEmulator.h>

#include <golem/audio/apu_emulator/AudioRegisters.h>

ChannelThreeEmulator::ChannelThreeEmulator(
    std::shared_ptr<AudioRegisters> audio_registers)
    : m_audio_registers(std::move(audio_registers))
    , m_on(false)
    , m_wave_index(0)
    , m_length_timer(0)
{
}

void ChannelThreeEmulator::getSamples(
    std::vector<float>& samples,
    size_t target_frequency)
{
    const bool dac_on = m_audio_registers->getChannel3On();

    // Verify DAC is on and we are currently playing a sound (after being
    // triggered) Verify also that volume is not muted
    if (!(dac_on && m_on) || m_volume == 0)
    {
        std::fill(samples.begin(), samples.end(), 0.f);
        return;
    }

    // Gameboy Frequency
    constexpr auto CPU_FREQUENCY = 4 * 1024 * 1024;
    const auto tick_increment = CPU_FREQUENCY / target_frequency;

    const auto wave_ram = m_audio_registers->getFlatWaveRam();
    const auto volume_scaling = m_volume == 1 ? 1 : m_volume == 2 ? 0.5 : 0.25;

    const auto length_timer_ticks = CPU_FREQUENCY / 256;

    const bool is_length_timer_enabled
        = m_audio_registers->getChannel3LengthEnable();

    for (size_t i = 0, end = samples.size(); i < end; ++i)
    {
        m_wave_ticks += tick_increment;

        const unsigned wave_index_increment
            = m_wave_ticks / (CPU_FREQUENCY / (m_frequency * 32));

        m_wave_ticks %= (CPU_FREQUENCY / (m_frequency * 32));
        m_wave_index = (m_wave_index + wave_index_increment) & 31;

        if (is_length_timer_enabled)
        {
            m_length_timer_ticks += tick_increment;
            const unsigned length_timer_increment
                = m_length_timer_ticks / length_timer_ticks;
            m_length_timer_ticks %= length_timer_ticks;
            m_length_timer = length_timer_increment > m_length_timer
                ? 0
                : m_length_timer - length_timer_increment;

            if (m_length_timer == 0)
            {
                m_on = false;
            }
        }

        const auto wave_value = wave_ram[m_wave_index];
        const auto output_value
            = (double(int(wave_value) - 8) / 7.) * volume_scaling;

        samples[i] = m_on ? output_value : 0.;
    }
}

void ChannelThreeEmulator::trigger()
{
    m_on = true;
    m_volume = m_audio_registers->getChannel3OutputLevel();
    m_wave_index = 0;
    const auto period = m_audio_registers->getChannel3Period();
    m_frequency = 65536 / (2048 - period);

    if (m_length_timer == 0)
    {
        m_length_timer = 256 - m_audio_registers->getChannel3LengthTimer();
    }
}
