#include <golem/use_cases/GetTrack.h>

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>
#include <golem/domain/Song.h>

namespace use_cases
{

GetTrack::GetTrack(domain::IProjectRepository& project_repository)
    : m_project_repository(project_repository)
{
}

const domain::Track* GetTrack::operator()(size_t song_index, size_t track_index)
{
    auto song = m_project_repository.getProject().song(song_index);

    if (song == nullptr)
    {
        return nullptr;
    }

    if (track_index >= domain::Song::TRACK_COUNT)
    {
        return nullptr;
    }

    return &song->getTrack(track_index);
}

}
