#pragma once

#include <filesystem>

namespace domain {
class Song;
}

namespace core {

class ISongSaver {
public:
    virtual ~ISongSaver() = default;

    virtual bool saveSong(
        const domain::Song& song,
        const std::filesystem::path& file_path)
        = 0;
};

}
