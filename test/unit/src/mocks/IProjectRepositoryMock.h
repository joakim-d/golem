#include <golem/domain/IProjectRepository.h>

#include <gmock/gmock.h>

class IProjectRepositoryMock : public domain::IProjectRepository {
public:
    MOCK_METHOD(domain::Project&, getProject, (), (override));
};
