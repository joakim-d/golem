#pragma once

#include <filesystem>

namespace domain {
class Song;
}

namespace core {

class ISongLoader {
public:
    virtual ~ISongLoader() = default;

    virtual bool loadSong(
        domain::Song& song,
        const std::filesystem::path& file_path)
        = 0;
};

}
