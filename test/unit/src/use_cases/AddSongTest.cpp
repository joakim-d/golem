#include <golem/use_cases/AddSong.h>

#include <golem/domain/Project.h>

#include <mocks/IProjectRepositoryMock.h>

#include <gtest/gtest.h>

namespace use_cases {

namespace {

    size_t INSTRUMENT_INDEX = 0;

}

TEST(AddSong, WhenAddingNewSong_ThenProjectContainsIt)
{
    IProjectRepositoryMock project_repository;

    domain::Project project;

    EXPECT_CALL(project_repository, getProject()).WillOnce(testing::ReturnRef(project));

    AddSong add_song(project_repository);

    add_song.execute();

    // Project will always have one instrument by default
    ASSERT_NE(project.song(1), nullptr);
}

}
