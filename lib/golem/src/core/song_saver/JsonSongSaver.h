#pragma once

#include <golem/core/ISongSaver.h>

namespace core::song_saver
{

class JsonSongSaver : public ISongSaver
{
public:
    bool saveSong(
        const domain::Song& song,
        const std::filesystem::path& file_path) override;
};

}
