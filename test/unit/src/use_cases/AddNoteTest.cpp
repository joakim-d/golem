#include <golem/use_cases/AddNote.h>

#include <golem/domain/Project.h>

#include <mocks/IProjectRepositoryMock.h>

#include <gtest/gtest.h>

namespace use_cases {

namespace {

    size_t SONG_INDEX = 0;
    size_t TRACK_INDEX = 0;
    size_t INSTRUMENT_INDEX = 0;

}

TEST(AddNote, WhenAddingNewNoteOnEmptyProject_ThenNoteIsAdded)
{
    IProjectRepositoryMock project_repository;

    domain::Project project;

    EXPECT_CALL(project_repository, getProject()).WillOnce(testing::ReturnRef(project));

    AddNote add_note(project_repository);

    const size_t note_index = 0;
    const size_t note_duration = 4;

    add_note.execute(SONG_INDEX,
        TRACK_INDEX,
        note_index,
        domain::Note { domain::NoteFrequency::A2, note_duration, INSTRUMENT_INDEX });

    auto song = project.song(SONG_INDEX);
    EXPECT_NE(song, nullptr);

    const auto note = song->getTrack(TRACK_INDEX).note(note_index);

    EXPECT_TRUE(note.has_value());
    EXPECT_EQ(note->duration(), note_duration);
    EXPECT_EQ(note->frequency(), domain::NoteFrequency::A2);
}

TEST(AddNote, WhenAddingNewNoteWithLongDuration_ThenFollowingNotesAreDeleted)
{
    IProjectRepositoryMock project_repository;

    domain::Project project;

    // Inserting two notes at index 3 and 4 with a duration of 1
    project.song(SONG_INDEX)
        ->getTrack(TRACK_INDEX)
        .setNote(3, domain::Note { domain::NoteFrequency::A2, 1, INSTRUMENT_INDEX });

    project.song(SONG_INDEX)
        ->getTrack(TRACK_INDEX)
        .setNote(4, domain::Note { domain::NoteFrequency::A2, 1, INSTRUMENT_INDEX });

    EXPECT_CALL(project_repository, getProject()).WillOnce(testing::ReturnRef(project));

    AddNote add_note(project_repository);

    const size_t note_index = 2;
    const size_t note_duration = 3;

    add_note.execute(SONG_INDEX,
        TRACK_INDEX,
        note_index,
        domain::Note { domain::NoteFrequency::A2, note_duration, INSTRUMENT_INDEX });

    EXPECT_FALSE(project.song(SONG_INDEX)->getTrack(TRACK_INDEX).note(3).has_value());
    EXPECT_FALSE(project.song(SONG_INDEX)->getTrack(TRACK_INDEX).note(4).has_value());
}

TEST(AddNote, WhenAddingNewNote_ThenPreviousOverlappingNoteHasItsDurationShortened)
{
    IProjectRepositoryMock project_repository;

    domain::Project project;

    // Inserting two notes at index 3 and 4 with a duration of 1
    project.song(SONG_INDEX)
        ->getTrack(TRACK_INDEX)
        .setNote(0, domain::Note { domain::NoteFrequency::A2, 8, INSTRUMENT_INDEX });

    EXPECT_CALL(project_repository, getProject()).WillOnce(testing::ReturnRef(project));

    AddNote add_note(project_repository);

    const size_t note_index = 4;
    const size_t note_duration = 1;

    add_note.execute(SONG_INDEX,
        TRACK_INDEX,
        note_index,
        domain::Note { domain::NoteFrequency::A2, note_duration, INSTRUMENT_INDEX });

    const auto first_note = project.song(SONG_INDEX)->getTrack(TRACK_INDEX).note(0);
    EXPECT_TRUE(first_note.has_value());

    EXPECT_EQ(first_note->duration(), 4);
}

}
