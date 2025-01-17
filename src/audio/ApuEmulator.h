#pragma once

#include <audio/apu_emulator/ChannelOneEmulator.h>
#include <audio/apu_emulator/ChannelThreeEmulator.h>
#include <audio/apu_emulator/ChannelTwoEmulator.h>

#include <core/IAudioProcessingUnit.h>

#include <memory>

class AudioRegisters;
class IAudioOutputBuffer;

namespace audio {

class ApuEmulator : public IAudioProcessingUnit {
public:
    ApuEmulator(
        std::shared_ptr<IAudioOutputBuffer> audio_output);

    void updateRegister(
        Register reg,
        uint8_t value,
        uint8_t mask) override;

    void updateWaveRam(
        const std::array<uint8_t, 16>& wave_ram) override;

private:
    void getSamplesCallback(char* data, size_t length);

    std::shared_ptr<IAudioOutputBuffer> m_audio_output;
    std::shared_ptr<AudioRegisters> m_audio_registers;
    ChannelOneEmulator m_channel_one_emulator;
    ChannelTwoEmulator m_channel_two_emulator;
    ChannelThreeEmulator m_channel_three_emulator;

    std::vector<float> m_channel_one_samples;
    std::vector<float> m_channel_two_samples;
    std::vector<float> m_channel_three_samples;
};

}
