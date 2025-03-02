#pragma once

#include <golem/core/IAudioOutputDevice.h>

#include <cstdint>

using SDL_AudioDeviceID = uint32_t;

namespace audio
{

class AudioOutput : public IAudioOutputDevice
{
public:
    AudioOutput();
    ~AudioOutput();

    size_t sampleRate() const override;
    size_t channelCount() const override;
    size_t frequency() const override;
    IAudioOutputDevice::AudioFormat format() const override;

    void play() override;
    void pause() override;

    void setGetSamplesCallback(std::function<void(char*, size_t length)>
                                   get_samples_callback) override;

private:
    std::function<void(char*, size_t length)> m_get_samples_callback;
    SDL_AudioDeviceID m_device_id;
};

}
