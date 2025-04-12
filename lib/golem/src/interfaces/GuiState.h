#pragma once

#include <cstddef>

namespace gui
{

class GuiState
{
public:
    static size_t songIndex();
    static void setSongIndex(size_t song_index);

    static size_t trackIndex();
    static void setTrackIndex(size_t track_index);

    static size_t instrumentIndex();
    static void setInstrumentIndex(size_t instrument_index);
};

}
