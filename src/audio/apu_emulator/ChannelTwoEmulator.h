#pragma once

#include <model/Note.h>

#include <memory>
#include <vector>

class AudioRegisters;

class ChannelTwoEmulator {
public:
    ChannelTwoEmulator(std::shared_ptr<AudioRegisters> audio_registers);

    void getSamples(std::vector<float>& samples);
    void trigger();

private:
    std::shared_ptr<AudioRegisters> m_audio_registers;
    size_t m_waveform_ticks;
    size_t m_volume_ticks;
    size_t m_length_timer_ticks;
    unsigned m_waveform_index;
    uint8_t m_volume;
    uint8_t m_length_timer;
    bool m_on;
};
