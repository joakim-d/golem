#pragma once

#include <memory>
#include <vector>

class AudioRegisters;

class ChannelThreeEmulator {
public:
    ChannelThreeEmulator(std::shared_ptr<AudioRegisters> audio_registers);

    void getSamples(
        std::vector<float>& samples,
        size_t target_frequency);

    void trigger();

private:
    std::shared_ptr<AudioRegisters> m_audio_registers;
    bool m_on;
    size_t m_wave_ticks;
    size_t m_length_timer_ticks;
    unsigned m_volume;
    unsigned m_wave_index;
    unsigned m_length_timer;
    unsigned m_frequency;
};
