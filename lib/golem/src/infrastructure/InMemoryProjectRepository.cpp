#include <golem/infrastructure/InMemoryProjectRepository.h>

namespace infrastructure
{

domain::Project& InMemoryProjectRepository::getProject()
{
    return m_project;
}

}
