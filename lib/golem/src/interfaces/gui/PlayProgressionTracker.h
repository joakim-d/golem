#pragma once

#include <golem/core/IPlayingProgressionTracker.h>

namespace gui
{

namespace views
{
    class MainView;
    class SongView;
}

class PlayProgressionTracker : public core::IPlayingProgressionTracker
{
public:
    PlayProgressionTracker(
        views::SongView& song_view,
        views::MainView& main_view);

    void onProgressMade(size_t tick_index, size_t note_index) override;

private:
    views::SongView& m_song_view;
    views::MainView& m_main_view;
};

}
