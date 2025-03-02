#pragma once

namespace domain {

class Project;

class IProjectRepository {
public:
    virtual ~IProjectRepository() = default;

    virtual Project& getProject() = 0;
};

}
