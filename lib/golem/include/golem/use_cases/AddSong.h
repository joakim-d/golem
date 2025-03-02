#pragma once

namespace domain {
class IProjectRepository;
}

namespace use_cases {

class AddSong {
public:
    AddSong(domain::IProjectRepository& project_repository);

    void execute();

private:
    domain::IProjectRepository& m_project_repository;
};

}
