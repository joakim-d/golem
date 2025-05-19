#include <golem/domain/events/ProjectDomainEventProcessor.h>
#include <golem/infrastructure/InMemoryProjectRepository.h>
#include <golem/infrastructure/exporter/CProjectExporter.h>
#include <golem/interfaces/gui/MainWindow.h>
#include <golem/use_cases/ProjectUseCases.h>

#include <golem/audio/ApuEmulator.h>
#include <golem/audio/AudioOutput.h>
#include <golem/core/NotePlayer.h>
#include <golem/core/SongPlayer.h>

#include <iostream>

int main(int argc, char* argv[])
{
    auto window = gui::MainWindow::create(800, 600, "[GoleM]");

    if (!window.has_value())
    {
        std::cerr << "Failed to create MainWindow" << std::endl;
        return -1;
    }

    infrastructure::InMemoryProjectRepository project_repository;
    domain::events::ProjectDomainEventProcessor events_processor;

    auto audio_output = std::make_shared<audio::AudioOutput>();
    auto apu_emulator = std::make_shared<audio::ApuEmulator>(audio_output);

    core::NotePlayer note_player { project_repository, apu_emulator };
    core::SongPlayer song_player { project_repository, events_processor,
                                   apu_emulator, note_player };

    std::map<domain::ExportFormat, domain::IExportService*> export_services;

    infrastructure::CProjectExporter cproject_exporter;
    export_services[domain::ExportFormat::CProject] = &cproject_exporter;

    use_cases::ProjectUseCases project_use_cases { project_repository,
                                                   events_processor,
                                                   note_player, song_player,
                                                   std::move(export_services) };

    audio_output->play();

    window->execute(project_use_cases, events_processor);

    return 0;
}
