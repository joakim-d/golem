#pragma once

#include <memory>
#include <vector>

class AudioRegisters;

class ChannelOneEmulator
{
public:
    ChannelOneEmulator(std::shared_ptr<AudioRegisters> audio_registers);

    void getSamples(std::vector<float>& samples);
    void trigger();

private:
    std::shared_ptr<AudioRegisters> m_audio_registers;
    unsigned m_waveform_ticks;
    unsigned m_volume_ticks;
    unsigned m_length_timer_ticks;
    unsigned m_sweep_ticks;
    unsigned m_waveform_index;
    uint8_t m_volume;
    uint8_t m_length_timer;
    bool m_on;
};
