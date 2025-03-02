#include <golem/domain/Project.h>

namespace domain
{

Project::Project()
{
    m_songs.push_back(Song {});
    m_instruments.push_back(Instrument {});
}

void Project::addSong(Song song)
{
    m_songs.push_back(std::move(song));
}

void Project::addInstrument(Instrument instrument)
{
    m_instruments.push_back(std::move(instrument));
}

Song* Project::song(size_t index)
{
    if (index >= m_songs.size())
    {
        return nullptr;
    }

    return &m_songs[index];
}

Instrument* Project::instrument(size_t index)
{
    if (index >= m_instruments.size())
    {
        return nullptr;
    }

    return &m_instruments[index];
}

const std::vector<Instrument>& Project::getInstruments() const
{
    return m_instruments;
}

}
