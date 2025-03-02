#include <golem/use_cases/ProjectUseCases.h>

namespace use_cases
{

ProjectUseCases::ProjectUseCases(
    domain::IProjectRepository& project_repository,
    domain::events::ProjectDomainEventProcessor& event_processor,
    core::NotePlayer& note_player,
    core::SongPlayer& song_player)
    : add_instrument(project_repository, event_processor)
    , add_note(project_repository)
    , add_song(project_repository)
    , create_new_project(project_repository)
    , get_instruments(project_repository)
    , get_phrase_view(project_repository)
    , get_playback_info(song_player)
    , get_notes_per_phrase(project_repository)
    , get_track(project_repository)
    , play_note(project_repository, note_player)
    , play_song(project_repository, song_player)
    , stop_note(project_repository, note_player)
    , stop_song(song_player)
{
}

}
