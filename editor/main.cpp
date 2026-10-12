// golem-editor [song.gsong] [--play] [--quit-after-frames N [--screenshot FILE.bmp]]
//
// The Golem editor: a tracker window to write songs and play them with the real driver in
// SameBoy. --play starts playback at once; --quit-after-frames exits after N frames (for the
// smoke test, with SDL_VIDEO_DRIVER=dummy and SDL_AUDIO_DRIVER=dummy), and --screenshot
// saves that last frame.

#include "app.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

namespace {

constexpr int kSampleRate = 44100;

// Samples between the one being rendered and the one being heard: those queued in the stream
// and the device's buffer (at its own rate).
std::size_t audio_latency(SDL_AudioStream* audio)
{
    std::size_t samples = static_cast<std::size_t>(std::max(SDL_GetAudioStreamQueued(audio), 0))
                        / sizeof(golem::StereoSample);
    SDL_AudioSpec device;
    int frames = 0;
    if (SDL_GetAudioDeviceFormat(SDL_GetAudioStreamDevice(audio), &device, &frames)
        && device.freq > 0) {
        samples +=
            static_cast<std::size_t>(frames) * kSampleRate / static_cast<std::size_t>(device.freq);
    }
    return samples;
}

// SDL asks for more audio on its own thread: the song and note previews are rendered there.
void SDLCALL feed_audio(
    void* userdata,
    SDL_AudioStream* stream,
    int additional_amount,
    int)
{
    auto* app = static_cast<golem::editor::App*>(userdata);
    static thread_local std::vector<golem::StereoSample> samples;
    samples.resize(static_cast<std::size_t>(additional_amount) / sizeof(golem::StereoSample));
    if (samples.empty()) {
        return;
    }
    app->render_audio(samples.data(), samples.size());
    SDL_PutAudioStreamData(
        stream, samples.data(), static_cast<int>(samples.size() * sizeof(golem::StereoSample)));
}

} // namespace

int main(
    int argc,
    char** argv)
{
    std::string song_path;
    bool play = false;
    long quit_after_frames = -1;
    std::string screenshot_path;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--play") {
            play = true;
        } else if (arg == "--quit-after-frames" && i + 1 < argc) {
            quit_after_frames = std::stol(argv[++i]);
        } else if (arg == "--screenshot" && i + 1 < argc) {
            screenshot_path = argv[++i];
        } else if (song_path.empty() && arg.rfind("--", 0) != 0) {
            song_path = arg;
        } else {
            std::fprintf(
                stderr,
                "usage: golem-editor [song.gsong] [--play] [--quit-after-frames N [--screenshot "
                "FILE]]\n");
            return 2;
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "golem-editor: cannot start SDL video: %s\n", SDL_GetError());
        return 1;
    }
    // With SDL's default 1024-frame buffer, PipeWire's PulseAudio server (0.3.x) can stop asking
    // for audio: silence, then a hang when the device closes. 2048 frames (46 ms) avoids it. The
    // SDL_AUDIO_DEVICE_SAMPLE_FRAMES environment variable still overrides this.
    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "2048");
    const bool has_audio = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (!has_audio) {
        std::fprintf(stderr, "golem-editor: no audio: %s\n", SDL_GetError());
    }

    SDL_Window* window =
        SDL_CreateWindow("Golem", 1200, 800, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (window == nullptr) {
        std::fprintf(stderr, "golem-editor: cannot create the window: %s\n", SDL_GetError());
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr) {
        // No accelerated renderer (e.g. headless video drivers): draw in software.
        std::fprintf(
            stderr, "golem-editor: no accelerated renderer (%s), using software\n", SDL_GetError());
        renderer = SDL_CreateRenderer(window, SDL_SOFTWARE_RENDERER);
    }
    if (renderer == nullptr) {
        std::fprintf(stderr, "golem-editor: cannot create a renderer: %s\n", SDL_GetError());
        return 1;
    }
    SDL_SetRenderVSync(renderer, 1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr; // No imgui.ini next to the songs.
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    {
        golem::editor::App app(window);
        if (!song_path.empty()) {
            app.open(song_path);
        }

        SDL_AudioStream* audio = nullptr;
        if (has_audio) {
            const SDL_AudioSpec spec = {SDL_AUDIO_S16, 2, kSampleRate};
            audio = SDL_OpenAudioDeviceStream(
                SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, feed_audio, &app);
            if (audio != nullptr) {
                SDL_ResumeAudioStreamDevice(audio);
            } else {
                std::fprintf(stderr, "golem-editor: no audio device: %s\n", SDL_GetError());
            }
        }
        if (play) {
            app.play();
        }

        for (long frame = 0; !app.quit_requested() && frame != quit_after_frames; ++frame) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL3_ProcessEvent(&event);
                app.handle_event(event);
            }
            if (audio != nullptr) {
                app.set_audio_latency(audio_latency(audio));
            }
            ImGui_ImplSDLRenderer3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            app.draw();
            ImGui::Render();
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
            if (!screenshot_path.empty() && frame + 1 == quit_after_frames) {
                if (SDL_Surface* shot = SDL_RenderReadPixels(renderer, nullptr)) {
                    SDL_SaveBMP(shot, screenshot_path.c_str());
                    SDL_DestroySurface(shot);
                }
            }
            SDL_RenderPresent(renderer);
        }

        if (audio != nullptr) {
            SDL_DestroyAudioStream(audio); // Before the player it feeds from goes away.
        }
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
