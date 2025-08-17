#pragma once

#include <string>

namespace domain
{

class Project;

enum class ExportFormat
{
    CProject
};

class IExportService
{
public:
    virtual ~IExportService() = default;

    virtual void
    exportProject(const Project& project, const std::string& filepath)
        = 0;
};

}
