#pragma once

#include <core/ISongLoader.h>

namespace core::song_loader {

class JsonSongLoader : public ISongLoader {
public:
    bool loadSong(
        model::Song& song,
        const std::filesystem::path& file_path) override;
};

}
