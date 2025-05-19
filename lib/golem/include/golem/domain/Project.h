#pragma once

#include <golem/domain/Pattern.h>
#include <golem/domain/Song.h>
#include <golem/domain/instruments/Instrument.h>

namespace domain
{

class Project
{
public:
    Project();

    void addSong(Song song);
    void addInstrument(Instrument instrument);
    void addPattern(Pattern pattern);

    Song* song(size_t index);
    Instrument* instrument(size_t index);
    Pattern* pattern(size_t index);

    size_t maxPatterns() const;
    void setMaxPatterns(size_t max_patterns);

    const std::vector<Instrument>& getInstruments() const;
    const std::vector<Song>& getSongs() const;
    const std::vector<Pattern>& getPatterns() const;

private:
    std::vector<Song> m_songs;
    std::vector<Instrument> m_instruments;
    std::vector<Pattern> m_patterns;
};

}
