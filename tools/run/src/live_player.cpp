#include "golem/live_player.h"

#include "golem/rom_runner.h"
#include "golem/song_binary.h"

#ifdef GOLEM_HAS_PLAYER_ROM
#include "sameboy_core.h"
#endif

#include <algorithm>
#include <mutex>

namespace golem {

#ifdef GOLEM_HAS_PLAYER_ROM
// Generated from driver/player.gb (driver/CMakeLists.txt, cmake/EmbedFile.cmake).
extern const unsigned char kPlayerRom[];
extern const std::size_t kPlayerRomSize;
#endif

namespace {

#ifdef GOLEM_HAS_PLAYER_ROM
    struct Destroy {
        void operator()(golem_sb* sb) const
        {
            golem_sb_destroy(sb);
        }
    };

    void collect(
        void* user,
        std::int16_t left,
        std::int16_t right)
    {
        static_cast<std::vector<StereoSample>*>(user)->push_back({left, right});
    }
#endif

    // Rendered samples are dropped from the front of the buffer once this many accumulate.
    constexpr std::size_t kCompactThreshold = 1 << 14;

} // namespace

struct LivePlayer::Impl {
    unsigned sample_rate;
    mutable std::mutex mutex;
#ifdef GOLEM_HAS_PLAYER_ROM
    std::vector<std::uint8_t> rom;
    std::unique_ptr<golem_sb, Destroy> sb;
#endif
    std::vector<StereoSample> pending; // Produced by SameBoy, not rendered yet.
    std::size_t next = 0; // First sample of `pending` not rendered yet.
};

std::string playback_unavailable_reason()
{
#if defined(GOLEM_HAS_PLAYER_ROM)
    return {};
#elif defined(GOLEM_HAS_SAMEBOY)
    return "the player ROM was not built, because RGBDS was not found";
#else
    return "SameBoy is not built in (it needs GCC or Clang)";
#endif
}

std::vector<std::uint8_t> player_rom(const Song& song)
{
#ifdef GOLEM_HAS_PLAYER_ROM
    const auto bytes = encode_song(song, kPlayerSongAddress);
    if (bytes.size() > kPlayerSongSlotSize) {
        throw RunError(
            "the song takes "
            + std::to_string(bytes.size())
            + " bytes, but the player ROM has room for "
            + std::to_string(kPlayerSongSlotSize));
    }
    std::vector<std::uint8_t> rom(kPlayerRom, kPlayerRom + kPlayerRomSize);
    std::copy(bytes.begin(), bytes.end(), rom.begin() + kPlayerSongAddress);
    return rom;
#else
    (void)song;
    throw RunError("cannot play songs: " + playback_unavailable_reason());
#endif
}

LivePlayer::LivePlayer(unsigned sample_rate)
    : impl_(std::make_unique<Impl>())
{
    impl_->sample_rate = sample_rate;
}

LivePlayer::~LivePlayer() = default;

void LivePlayer::play(const Song& song)
{
#ifdef GOLEM_HAS_PLAYER_ROM
    auto rom = player_rom(song);
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->sb.reset();
    impl_->rom = std::move(rom);
    impl_->pending.clear();
    impl_->next = 0;
    const char* error = nullptr;
    impl_->sb.reset(
        golem_sb_create(impl_->rom.data(), impl_->rom.size(), nullptr, nullptr, &error));
    if (!impl_->sb) {
        throw RunError(error);
    }
    golem_sb_set_audio(impl_->sb.get(), impl_->sample_rate, collect, &impl_->pending);
#else
    player_rom(song); // Throws: playback is not built in.
#endif
}

void LivePlayer::stop()
{
    const std::lock_guard<std::mutex> lock(impl_->mutex);
#ifdef GOLEM_HAS_PLAYER_ROM
    impl_->sb.reset();
#endif
    impl_->pending.clear();
    impl_->next = 0;
}

bool LivePlayer::is_playing() const
{
#ifdef GOLEM_HAS_PLAYER_ROM
    const std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->sb != nullptr;
#else
    return false;
#endif
}

void LivePlayer::render(
    StereoSample* samples,
    std::size_t count)
{
    const std::lock_guard<std::mutex> lock(impl_->mutex);
#ifdef GOLEM_HAS_PLAYER_ROM
    if (impl_->sb) {
        const char* error = nullptr;
        while (impl_->pending.size() - impl_->next < count) {
            golem_sb_run_frame(impl_->sb.get(), &error);
        }
        std::copy_n(
            impl_->pending.begin() + static_cast<std::ptrdiff_t>(impl_->next), count, samples);
        impl_->next += count;
        if (impl_->next >= kCompactThreshold) {
            impl_->pending.erase(
                impl_->pending.begin(),
                impl_->pending.begin() + static_cast<std::ptrdiff_t>(impl_->next));
            impl_->next = 0;
        }
        return;
    }
#endif
    (void)kCompactThreshold;
    std::fill_n(samples, count, StereoSample {});
}

} // namespace golem
