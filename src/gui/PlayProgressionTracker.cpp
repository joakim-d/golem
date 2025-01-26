#include <gui/PlayProgressionTracker.h>

#include <gui/views/MainView.h>
#include <gui/views/SongView.h>

namespace gui {

PlayProgressionTracker::PlayProgressionTracker(
    views::SongView& song_view,
    views::MainView& main_view)
    : m_song_view(song_view)
    , m_main_view(main_view)
{
}

void PlayProgressionTracker::onProgressMade(
    size_t tick_index,
    size_t note_index,
    size_t phrase_index)
{
    m_song_view.onProgressionChanged(tick_index, note_index, phrase_index);
    m_main_view.onProgressionChanged(tick_index, note_index, phrase_index);
}

}
