#include <golem/domain/Project.h>

namespace domain
{

Project::Project()
{
    Song song {};
    song.setName("Song 1");
    m_songs.push_back(std::move(song));

    auto instrument = Instrument {};
    instrument.setName("Instrument 1");
    m_instruments.push_back(std::move(instrument));

    Pattern pattern { 16 };
    m_patterns.resize(16, pattern);
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

Pattern* Project::pattern(size_t index)
{
    if (index >= m_patterns.size())
    {
        return nullptr;
    }

    return &m_patterns[index];
}

const std::vector<Instrument>& Project::getInstruments() const
{
    return m_instruments;
}

const std::vector<Song>& Project::getSongs() const
{
    return m_songs;
}

size_t Project::maxPatterns() const
{
    return m_patterns.size();
}

void Project::setMaxPatterns(size_t max_patterns)
{
    m_patterns.resize(max_patterns, Pattern { 16 });
}

}
