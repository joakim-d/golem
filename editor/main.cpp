// golem-editor [song.gsong] [--play] [--quit-after-frames N [--screenshot FILE.bmp]]
//
// The Golem editor: a tracker window to write songs and play them with the real driver in
// SameBoy. --play starts playback at once; --quit-after-frames exits after N frames (for the
// smoke test, with SDL_VIDEO_DRIVER=offscreen and SDL_AUDIO_DRIVER=dummy), and --screenshot
// saves that last frame.

#include "app.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

#include <cstdio>
#include <string>
#include <vector>

namespace {

constexpr int kSampleRate = 44100;

// SDL asks for more audio on its own thread: the song is rendered there by LivePlayer.
void SDLCALL feed_audio(
    void* userdata,
    SDL_AudioStream* stream,
    int additional_amount,
    int)
{
    auto* player = static_cast<golem::LivePlayer*>(userdata);
    static thread_local std::vector<golem::StereoSample> samples;
    samples.resize(static_cast<std::size_t>(additional_amount) / sizeof(golem::StereoSample));
    if (samples.empty()) {
        return;
    }
    player->render(samples.data(), samples.size());
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
        std::fprintf(stderr, "golem-editor: %s\n", SDL_GetError());
        return 1;
    }
    const bool has_audio = SDL_InitSubSystem(SDL_INIT_AUDIO);
    if (!has_audio) {
        std::fprintf(stderr, "golem-editor: no audio: %s\n", SDL_GetError());
    }

    SDL_Window* window =
        SDL_CreateWindow("Golem", 1200, 800, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    SDL_Renderer* renderer = window ? SDL_CreateRenderer(window, nullptr) : nullptr;
    if (renderer == nullptr) {
        std::fprintf(stderr, "golem-editor: %s\n", SDL_GetError());
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
                SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, feed_audio, &app.player());
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
