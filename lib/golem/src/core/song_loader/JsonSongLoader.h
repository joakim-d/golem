#pragma once

#include <golem/core/ISongLoader.h>

namespace core::song_loader
{

class JsonSongLoader : public ISongLoader
{
public:
    bool loadSong(domain::Song& song, const std::filesystem::path& file_path)
        override;
};

}
