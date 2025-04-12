#include <interfaces/GuiState.h>

namespace gui
{

static size_t SONG_INDEX { 0 };
static size_t TRACK_INDEX { 0 };
static size_t INSTRUMENT_INDEX { 0 };

size_t GuiState::songIndex()
{
    return SONG_INDEX;
}

void GuiState::setSongIndex(size_t index)
{
    SONG_INDEX = index;
}

size_t GuiState::trackIndex()
{
    return TRACK_INDEX;
}

void GuiState::setTrackIndex(size_t index)
{
    TRACK_INDEX = index;
}

size_t GuiState::instrumentIndex()
{
    return INSTRUMENT_INDEX;
}

void GuiState::setInstrumentIndex(size_t index)
{
    INSTRUMENT_INDEX = index;
}

}
