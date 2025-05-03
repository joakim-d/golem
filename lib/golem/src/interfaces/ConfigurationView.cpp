#include <interfaces/ConfigurationView.h>

#include <golem/utils/scope_guard.h>

#include <interfaces/ImGuiTools.h>

#include <golem/domain/Song.h>
#include <golem/domain/instruments/Instrument.h>
#include <golem/use_cases/ProjectUseCases.h>

#include <imgui.h>

#include <interfaces/GuiState.h>

namespace gui
{

static void renderSongs(use_cases::ProjectUseCases& project_use_cases)
{
    const auto& songs = project_use_cases.get_songs();

    if (ImGui::Button("Add Song", { -1, 0.f }))
    {
        project_use_cases.add_song.execute();
    }

    if (ImGui::CollapsingHeader("Songs", ImGuiTreeNodeFlags_DefaultOpen))
    {
        unsigned index = 0;
        const auto song_index = GuiState::songIndex();
        char buffer[32];
        for (const auto& song : songs)
        {
            ImGui::PushID(index);

            snprintf(buffer, sizeof(buffer), "Song %u", index + 1);
            ImGui::TreeNodeEx(
                "Configuration##songs",
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen
                    | (index == song_index ? ImGuiTreeNodeFlags_Selected
                                           : ImGuiTreeNodeFlags_None),
                "%s", song.name().c_str());

            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
                GuiState::setSongIndex(index);
            }
            if (ImGui::BeginPopupContextItem())
            {
                char buffer[64] {};
                strncpy(buffer, song.name().c_str(), sizeof(buffer) - 1);
                ImGui::Text("Song Configuration");

                if (ImGui::InputText("Song Name", buffer, sizeof(buffer)))
                {
                    project_use_cases.set_song_name(index, buffer);
                }

                int ticks_per_note
                    = project_use_cases.get_ticks_per_note(index);

                if (ImGui::InputInt("Ticks per note", &ticks_per_note))
                {
                    project_use_cases.set_ticks_per_note(index, ticks_per_note);
                }

                ImGui::EndPopup();
            }

            ++index;
            ImGui::PopID();
        }
    }
}

static void renderInstruments(use_cases::ProjectUseCases& project_use_cases)
{
    if (ImGui::Button("Add Instrument", { -1, 0.f }))
    {
        project_use_cases.add_instrument.execute();
    }

    const auto& instruments = project_use_cases.get_instruments();

    if (ImGui::CollapsingHeader("Instruments", ImGuiTreeNodeFlags_DefaultOpen))
    {
        unsigned index = 0;
        const auto instrument_index = GuiState::instrumentIndex();
        char buffer[32];
        for (const auto& instrument : instruments)
        {
            ImGui::PushID(index);
            snprintf(buffer, sizeof(buffer), "Instrument %u", index + 1);
            ImGui::TreeNodeEx(
                "Configuration##instruments",
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen
                    | (index == instrument_index ? ImGuiTreeNodeFlags_Selected
                                                 : ImGuiTreeNodeFlags_None),
                "%s", instrument.name().c_str());

            if (ImGui::IsItemClicked())
            {
                GuiState::setInstrumentIndex(index);
            }

            if (ImGui::BeginPopupContextItem())
            {
                char buffer[64] {};
                strncpy(buffer, instrument.name().c_str(), sizeof(buffer) - 1);
                ImGui::Text("Instrument Configuration");

                if (ImGui::InputText("Instrument Name", buffer, sizeof(buffer)))
                {
                    project_use_cases.set_instrument_name(index, buffer);
                }

                static const char* items[] = { "12.5%", "25%", "50%", "75%" };
                int duty_cycle
                    = static_cast<int>(instrument.pulse().dutyCycle());

                if (ImGui::Combo(
                        "Duty Cycle", &duty_cycle, items, IM_ARRAYSIZE(items)))
                {
                    auto pulse = instrument.pulse();
                    pulse.setDutyCycle(
                        static_cast<domain::PulseInstrument::DutyCycle>(
                            duty_cycle));
                    project_use_cases.set_instrument_pulse(index, pulse);
                }

                ImGui::EndPopup();
            }

            ++index;
            ImGui::PopID();
        }
    }
}

ConfigurationView::ConfigurationView(
    use_cases::ProjectUseCases& project_use_cases)
{
    ImGui::BeginChild(
        "ConfigurationView", ImVec2 { -1, -1 }, 0,
        ImGuiWindowFlags_NoScrollbar);

    utils::ScopeGuard scope_guard { ImGui::EndChild };

    renderSongs(project_use_cases);
    renderInstruments(project_use_cases);

    ImGui::Dummy({ -1, -1 });
}

}
