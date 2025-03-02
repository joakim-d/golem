#include <core/song_saver/JsonSongSaver.h>

#include <golem/domain/Song.h>

#include <nlohmann/json.hpp>

#include <fstream>
#include <iostream>

namespace core::song_saver
{

static void savePhrases(const domain::Song& song, nlohmann::json& json)
{
    auto phrases = nlohmann::json::array();

    //     song.visitPhrases([&phrases](const domain::Phrase& phrase) {
    //         auto json_phrase = nlohmann::json::array();
    //         auto json_note = nlohmann::json::object();
    //         for (auto note_index = 0u; note_index <
    //         domain::Phrase::NOTE_COUNT; ++note_index) {
    //             const auto note_value = phrase.note(note_index);
    //             if (note_value.has_value()) {
    //                 json_note["frequency"] = note_value->frequency();
    //                 json_note["duration"] = note_value->duration();
    //                 json_note["instrument_index"] =
    //                 note_value->instrumentIndex();
    //             } else {
    //                 json_note["frequency"] = nullptr;
    //                 json_note["duration"] = nullptr;
    //                 json_note["instrument_index"] = nullptr;
    //             }
    //             json_phrase.push_back(json_note);
    //         }
    //         phrases.push_back(json_phrase);
    //     });

    //     json["song"]["phrases"] = phrases;
    // }

    // static void saveInstruments(
    //     const domain::Song& song,
    //     nlohmann::json& json)
    // {
    //     auto instruments = nlohmann::json::array();

    //     song.visitInstruments([&instruments](const domain::Instrument&
    //     instrument) {
    //         auto json_instrument = nlohmann::json::object();
    //         json_instrument["name"] = instrument.name();
    //         json_instrument["type"] = instrument.type();
    //         auto json_pulse = nlohmann::json::object();
    //         const auto& pulse = instrument.pulse();

    //         json_pulse["duty_cycle"] = pulse.dutyCycle();
    //         json_pulse["envelope_increase"] = pulse.envelopeIncrease();
    //         json_pulse["envelope_initial_volume"] =
    //         pulse.envelopeInitialVolume(); json_pulse["envelope_pace"] =
    //         pulse.envelopePace(); json_pulse["initial_length_timer"] =
    //         pulse.initialLengthTimer(); json_pulse["length_enable"] =
    //         pulse.lengthEnable(); json_pulse["sweep_increase"] =
    //         pulse.sweepIncrease(); json_pulse["sweep_pace"] =
    //         pulse.sweepPace(); json_pulse["sweep_steps"] =
    //         pulse.sweepSteps(); json_instrument["pulse"] = json_pulse;

    //         auto json_wave = nlohmann::json::object();
    //         const auto& wave = instrument.wave();
    //         json_wave["length_enable"] = wave.lengthEnable();
    //         json_wave["length_timer"] = wave.lengthTimer();
    //         json_wave["volume"] = wave.volume();
    //         auto json_samples = nlohmann::json::array();
    //         for (auto sample : wave.getSamples()) {
    //             json_samples.push_back(sample);
    //         }
    //         json_wave["samples"] = json_samples;
    //         json_instrument["wave"] = json_wave;

    //         instruments.push_back(json_instrument);
    //     });

    //     json["song"]["instruments"] = instruments;
}

void saveTracks(const domain::Song& song, nlohmann::json& json)
{
    // auto tracks = nlohmann::json::array();

    // song.visitTracks(
    //     [&tracks](const domain::Track& track)
    //     {
    //         auto json_track = nlohmann::json::array();
    //         auto json_chain = nlohmann::json::object();

    //         for (size_t i = 0; i < domain::Track::PHRASE_COUNT; ++i)
    //         {
    //             const auto chain_index = track.phraseIndex(i);
    //             if (chain_index.has_value())
    //             {
    //                 json_chain["chain_index"] = *chain_index;
    //             }
    //             else
    //             {
    //                 json_chain["chain_index"] = nullptr;
    //             }
    //             json_track.push_back(json_chain);
    //         }
    //         tracks.push_back(json_track);
    //     });

    // json["song"]["tracks"] = tracks;
}

bool JsonSongSaver::saveSong(
    const domain::Song& song,
    const std::filesystem::path& file_path)
{
    // std::ofstream file(file_path);

    // if (!file.is_open())
    // {
    //     std::cerr << "Unable to open file for writing\n";
    //     return false;
    // }

    // nlohmann::json json;
    // json["song"] = nlohmann::json::object();

    // savePhrases(song, json);
    // saveInstruments(song, json);
    // saveTracks(song, json);

    // file << json.dump(4);
    // file.close();

    return true;
}

}
