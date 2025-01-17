#pragma once

#include <filesystem>

namespace model {
class Song;
}

namespace core {

class ISongLoader {
public:
    virtual ~ISongLoader() = default;

    virtual bool loadSong(
        model::Song& song,
        const std::filesystem::path& file_path)
        = 0;
};

}
