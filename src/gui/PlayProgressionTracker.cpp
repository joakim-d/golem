#include <gui/PlayProgressionTracker.h>

#include <gui/views/SongView.h>

namespace gui {

PlayProgressionTracker::PlayProgressionTracker(
    views::SongView& song_view)
    : m_song_view(song_view)
{
}

void PlayProgressionTracker::onProgressMade(
    size_t note_index,
    size_t phrase_index)
{
    m_song_view.onProgressionChanged(note_index, phrase_index);
}

}
