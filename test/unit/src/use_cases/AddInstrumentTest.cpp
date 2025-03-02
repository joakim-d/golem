#include <golem/use_cases/AddInstrument.h>

#include <golem/domain/Project.h>

#include <mocks/IProjectRepositoryMock.h>

#include <gtest/gtest.h>

namespace use_cases {

namespace {

    size_t INSTRUMENT_INDEX = 0;

}

TEST(AddInstrument, WhenAddingNewInstrument_ThenProjectContainsIt)
{
    IProjectRepositoryMock project_repository;

    domain::Project project;

    EXPECT_CALL(project_repository, getProject()).WillOnce(testing::ReturnRef(project));

    AddInstrument add_instrument(project_repository);

    add_instrument.execute();

    // Project will always have one instrument by default
    ASSERT_NE(project.instrument(1), nullptr);
}

}
