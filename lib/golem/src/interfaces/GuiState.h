#pragma once

#include <cstddef>
#include <imgui.h>

namespace gui {

class GuiState {
public:
	static ImColor selectedColor();
	static ImColor idleColor();
    static ImColor hoveredColor();

    static size_t songIndex();
    static void setSongIndex(size_t song_index);

    static size_t trackIndex();
    static void setTrackIndex(size_t track_index);

    static size_t instrumentIndex();
    static void setInstrumentIndex(size_t instrument_index);

    static size_t patternIndex();
	static void setPatternIndex(size_t pattern_index);
};

}
