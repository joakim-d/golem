#pragma once

#include <core/ISongSaver.h>

namespace core::song_saver {

class JsonSongSaver : public ISongSaver {
public:
    bool saveSong(
        const model::Song& song,
        const std::filesystem::path& file_path)
        override;
};

}
