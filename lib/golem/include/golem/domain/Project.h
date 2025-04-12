#pragma once

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

    Song* song(size_t index);
    Instrument* instrument(size_t index);

    const std::vector<Instrument>& getInstruments() const;
    const std::vector<Song>& getSongs() const;

private:
    std::vector<Song> m_songs;
    std::vector<Instrument> m_instruments;
};

}
