#include <audio/AudioOutput.h>

#include <SDL2/SDL.h>

#include <iostream>

namespace audio {

static void custom_get_samples_callback(void* parameters, Uint8* data, int length)
{
    auto callback = (std::function<void(char*, size_t length)>*)(parameters);
    (*callback)(reinterpret_cast<char*>(data), length);
}

AudioOutput::AudioOutput()
{
    auto result = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (result != 0) {
        std::cerr << "Failed to init SDL_AUDIO" << std::endl;
    }
}

AudioOutput::~AudioOutput()
{
    SDL_CloseAudioDevice(m_device_id);
}

size_t AudioOutput::sampleRate() const
{
    return 4096;
}

size_t AudioOutput::channelCount() const
{
    return 2;
}

size_t AudioOutput::frequency() const
{
    return 44100;
}

IAudioOutputBuffer::AudioFormat AudioOutput::format() const
{
    return AudioFormat::Int16;
}

void AudioOutput::setGetSamplesCallback(
    std::function<void(char*, size_t length)> get_samples_callback)
{
    m_get_samples_callback = std::move(get_samples_callback);

    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = AUDIO_S16;
    spec.channels = 2;
    spec.samples = 1024;
    spec.userdata = &m_get_samples_callback;
    spec.callback = custom_get_samples_callback;

    m_device_id = SDL_OpenAudioDevice(
        nullptr,
        0,
        &spec,
        &spec,
        0);

    if (m_device_id == 0) {
        char buffer[256];
        std::cerr << SDL_GetErrorMsg(buffer, 255) << std::endl;
        return;
    }

    SDL_PauseAudioDevice(m_device_id, SDL_FALSE);
}

}
