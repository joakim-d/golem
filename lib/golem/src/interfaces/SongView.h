#pragma once

#include <cstddef>

namespace use_cases
{
class ProjectUseCases;
}

namespace gui
{

class SongView
{
public:
    SongView(use_cases::ProjectUseCases& use_cases, size_t m_song_index);
};

}
