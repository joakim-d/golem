#pragma once

#include <core/IAudioOutputBuffer.h>

#include <cstdint>

using SDL_AudioDeviceID = uint32_t;

namespace audio {

class AudioOutput : public IAudioOutputBuffer {
public:
    AudioOutput();
    ~AudioOutput();
    size_t sampleRate() const override;
    size_t channelCount() const override;
    size_t frequency() const override;
    IAudioOutputBuffer::AudioFormat format() const override;

    void setGetSamplesCallback(
        std::function<void(char*, size_t length)> get_samples_callback)
        override;

private:
    std::function<void(char*, size_t length)> m_get_samples_callback;
    SDL_AudioDeviceID m_device_id;
};

}
