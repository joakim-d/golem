#pragma once

#include <golem/domain/Note.h>

#include <memory>
#include <vector>

class AudioRegisters;

class ChannelTwoEmulator
{
public:
    ChannelTwoEmulator(std::shared_ptr<AudioRegisters> audio_registers);

    void getSamples(std::vector<float>& samples);
    void trigger();

private:
    std::shared_ptr<AudioRegisters> m_audio_registers;
    unsigned m_waveform_ticks;
    unsigned m_volume_ticks;
    unsigned m_length_timer_ticks;
    unsigned m_waveform_index;
    uint8_t m_volume;
    uint8_t m_length_timer;
    bool m_on;
};
