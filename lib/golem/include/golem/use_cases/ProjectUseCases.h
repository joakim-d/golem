#pragma once

#include <golem/use_cases/AddInstrument.h>
#include <golem/use_cases/AddNote.h>
#include <golem/use_cases/AddSong.h>
#include <golem/use_cases/CreateNewProject.h>
#include <golem/use_cases/ExportProject.h>
#include <golem/use_cases/GetInstruments.h>
#include <golem/use_cases/GetMinMaxTrackFrequencies.h>
#include <golem/use_cases/GetNotesPerPhrase.h>
#include <golem/use_cases/GetPattern.h>
#include <golem/use_cases/GetPlaybackInfo.h>
#include <golem/use_cases/GetSong.h>
#include <golem/use_cases/GetSongs.h>
#include <golem/use_cases/GetTicksPerNote.h>
#include <golem/use_cases/GetTrack.h>
#include <golem/use_cases/PlayNote.h>
#include <golem/use_cases/PlaySong.h>
#include <golem/use_cases/SetInstrumentName.h>
#include <golem/use_cases/SetInstrumentPulse.h>
#include <golem/use_cases/SetSongName.h>
#include <golem/use_cases/SetTicksPerNote.h>
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
        core::SongPlayer& song_player,
        std::map<domain::ExportFormat, domain::IExportService*>
            export_services);

    AddInstrument add_instrument;
    AddNote add_note;
    AddSong add_song;
    CreateNewProject create_new_project;
    ExportProject export_project;
    GetInstruments get_instruments;
    GetPlaybackInfo get_playback_info;
    GetMinMaxTrackFrequencies get_min_max_track_frequencies;
    GetNotesPerPhrase get_notes_per_phrase;
    GetPattern get_pattern;
    GetSong get_song;
    GetSongs get_songs;
    GetTicksPerNote get_ticks_per_note;
    GetTrack get_track;
    PlayNote play_note;
    PlaySong play_song;
    SetInstrumentName set_instrument_name;
    SetInstrumentPulse set_instrument_pulse;
    SetSongName set_song_name;
    SetTicksPerNote set_ticks_per_note;
    StopNote stop_note;
    StopSong stop_song;
};

}
