#include <golem/use_cases/ExportProject.h>

#include <golem/domain/IExportService.h>
#include <golem/domain/IProjectRepository.h>

namespace use_cases
{

ExportProject::ExportProject(
    domain::IProjectRepository& project_repository,
    std::map<domain::ExportFormat, domain::IExportService*> export_services)
    : m_repository(project_repository)
    , m_export_services(std::move(export_services))
{
}

void ExportProject::operator()(
    domain::ExportFormat format,
    const std::string& filepath)
{
    auto service_iterator = m_export_services.find(format);

    if (service_iterator == m_export_services.end())
    {
        return;
    }

    service_iterator->second->exportProject(
        m_repository.getProject(), filepath);
}

}
