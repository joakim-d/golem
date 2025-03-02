#pragma once

#include <golem/domain/IProjectRepository.h>
#include <golem/domain/Project.h>

namespace infrastructure
{

class InMemoryProjectRepository : public domain::IProjectRepository
{
public:
    virtual domain::Project& getProject() override;

private:
    domain::Project m_project;
};

}
