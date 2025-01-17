#pragma once

#include <core/IPlayingProgressionTracker.h>

namespace gui {

namespace views {
    class SongView;
}

class PlayProgressionTracker : public core::IPlayingProgressionTracker {
public:
    PlayProgressionTracker(views::SongView& song_view);

    void onProgressMade(size_t note_index, size_t phrase_index) override;

private:
    views::SongView& m_song_view;
};

}
