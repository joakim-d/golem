#pragma once

#include <golem/use_cases/AddInstrument.h>
#include <golem/use_cases/AddNote.h>
#include <golem/use_cases/AddSong.h>
#include <golem/use_cases/CreateNewProject.h>
#include <golem/use_cases/GetInstruments.h>
#include <golem/use_cases/GetNotesPerPhrase.h>
#include <golem/use_cases/GetPhraseView.h>
#include <golem/use_cases/GetPlaybackInfo.h>
#include <golem/use_cases/GetTrack.h>
#include <golem/use_cases/PlayNote.h>
#include <golem/use_cases/PlaySong.h>
#include <golem/use_cases/StopNote.h>
#include <golem/use_cases/StopSong.h>

namespace core
{
class NotePlayer;
}

namespace use_cases
{

class ProjectUseCases
{
public:
    ProjectUseCases(
        domain::IProjectRepository& project_repository,
        domain::events::ProjectDomainEventProcessor& events_processor,
        core::NotePlayer& note_player,
        core::SongPlayer& song_player);

    AddInstrument add_instrument;
    AddNote add_note;
    AddSong add_song;
    CreateNewProject create_new_project;
    GetInstruments get_instruments;
    GetPhraseView get_phrase_view;
    GetPlaybackInfo get_playback_info;
    GetNotesPerPhrase get_notes_per_phrase;
    GetTrack get_track;
    PlayNote play_note;
    PlaySong play_song;
    StopNote stop_note;
    StopSong stop_song;
};

}
