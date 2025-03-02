#pragma once

namespace domain
{
class IProjectRepository;
}

namespace use_cases
{

class CreateNewProject
{
public:
    CreateNewProject(domain::IProjectRepository& project_repository);

    void execute();

private:
    domain::IProjectRepository& m_project_repository;
};

} // namespace use_cases
