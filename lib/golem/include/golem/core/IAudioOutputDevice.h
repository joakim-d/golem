#pragma once

#include <cstddef>
#include <functional>

class IAudioOutputDevice {
public:
    enum class AudioFormat {
        Unknown,
        Uint8,
        Int16,
        Int32,
        Float
    };

    virtual ~IAudioOutputDevice() = default;

    virtual size_t sampleRate() const = 0;
    virtual size_t channelCount() const = 0;
    virtual size_t frequency() const = 0;
    virtual AudioFormat format() const = 0;

    virtual void play() = 0;
    virtual void pause() = 0;

    virtual void setGetSamplesCallback(
        std::function<void(char*, size_t length)> get_samples_callback)
        = 0;

    static constexpr size_t byteCountForFormat(AudioFormat format)
    {
        switch (format) {
        case AudioFormat::Unknown:
            return 0;
        case AudioFormat::Uint8:
            return 1;
        case AudioFormat::Int16:
            return 2;
        default:
            return 4;
        }
    }
};
