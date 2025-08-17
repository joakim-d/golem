#pragma once

#include <string>

#include <map>

namespace domain
{
enum class ExportFormat;
class IExportService;
class IProjectRepository;
}

namespace use_cases
{

class ExportProject
{
public:
    ExportProject(
        domain::IProjectRepository& project_repository,
        std::map<domain::ExportFormat, domain::IExportService*> export_service);

    void operator()(domain::ExportFormat format, const std::string& filepath);

private:
    domain::IProjectRepository& m_repository;
    std::map<domain::ExportFormat, domain::IExportService*> m_export_services;
};

}
