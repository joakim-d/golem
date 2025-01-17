#pragma once

#include <filesystem>

namespace model {
class Song;
}

namespace core {

class ISongSaver {
public:
    virtual ~ISongSaver() = default;

    virtual bool saveSong(
        const model::Song& song,
        const std::filesystem::path& file_path)
        = 0;
};

}
