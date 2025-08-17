#pragma once

#include <golem/domain/IExportService.h>

namespace infrastructure
{

class CProjectExporter : public domain::IExportService
{
public:
    void exportProject(
        const domain::Project& project,
        const std::string& filepath) override;
};

}
