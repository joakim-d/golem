#include <core/song_loader/JsonSongLoader.h>

#include <golem/domain/Song.h>

#include <nlohmann/json.hpp>

#include <fstream>

#include <iostream>

namespace core::song_loader
{

template <typename T>
std::optional<T>
getJsonValue(const nlohmann::json& json, const std::string& key)
{
    auto key_value = json.find(key);
    if (key_value == json.end())
    {
        return {};
    }
    if (key_value->is_null())
    {
        return {};
    }
    return *key_value;
}

template <typename T>
T getJsonValueOrDefault(
    const nlohmann::json& json,
    const std::string& key,
    T default_value)
{
    auto key_value = json.find(key);
    if (key_value == json.end())
    {
        return default_value;
    }
    if (key_value->is_null())
    {
        return default_value;
    }
    return *key_value;
}

static void loadPhrases(domain::Song& song, const nlohmann::json& json_root)
{
    size_t phrase_index = 0;

    for (const auto& phrase : json_root["song"]["phrases"])
    {
        // auto& phrase_model = song.getPhrase(phrase_index);
        // size_t note_index = 0;
        // for (const auto& note : phrase) {
        //     const auto frequency = getJsonValue<domain::NoteFrequency>(note,
        //     "frequency"); const auto instrument_index =
        //     getJsonValue<size_t>(note, "instrument_index"); const auto
        //     duration = getJsonValue<size_t>(note, "duration"); if (frequency
        //     && instrument_index && duration) {
        //         phrase_model.setNote(
        //             note_index,
        //             domain::Note {
        //                 *frequency,
        //                 *duration,
        //                 *instrument_index });
        //     }
        //     ++note_index;
        //     if (note_index == domain::Phrase::NOTE_COUNT) {
        //         break;
        //     }
        // }
        // ++phrase_index;
        // if (phrase_index == domain::Song::PHRASE_COUNT) {
        //     break;
        // }
    }
}

static void loadInstruments(domain::Song& song, const nlohmann::json& json_root)
{
    // size_t instrument_index = 0;

    // for (const auto& instrument : json_root["song"]["instruments"])
    // {
    //     auto& instrument_model = song.getInstrument(instrument_index);

    //     instrument_model.setName(
    //         getJsonValueOrDefault<std::string>(instrument, "name", ""));

    //     instrument_model.setType(
    //         getJsonValueOrDefault<domain::Instrument::Type>(
    //             instrument, "type", domain::Instrument::Type::None));

    //     auto& pulse_model = instrument_model.pulse();

    //     if (instrument.find("pulse") != instrument.end())
    //     {
    //         auto& pulse = instrument["pulse"];
    //         pulse_model.setDutyCycle(getJsonValueOrDefault(
    //             pulse, "duty_cycle",
    //             domain::PulseInstrument::DutyCycle::Percent_12_5));

    //         pulse_model.setEnvelopeIncrease(
    //             getJsonValueOrDefault(pulse, "envelope_increase", false));
    //         pulse_model.setEnvelopePace(
    //             getJsonValueOrDefault(pulse, "envelope_pace", 0));
    //         pulse_model.setEnvelopeInitialVolume(
    //             getJsonValueOrDefault(pulse, "envelope_initial_volume", 0));
    //         pulse_model.setInitialLengthTimer(
    //             getJsonValueOrDefault(pulse, "initial_length_timer", 0));
    //         pulse_model.setLengthEnable(
    //             getJsonValueOrDefault(pulse, "length_enable", false));
    //         pulse_model.setSweepIncrease(
    //             getJsonValueOrDefault(pulse, "sweep_increase", false));
    //         pulse_model.setSweepPace(
    //             getJsonValueOrDefault(pulse, "sweep_pace", 0));
    //         pulse_model.setSweepSteps(
    //             getJsonValueOrDefault(pulse, "sweep_steps", 0));
    //     }

    //     if (instrument.find("wave") != instrument.end())
    //     {
    //         auto& wave = instrument["wave"];
    //         auto& wave_model = instrument_model.wave();

    //         wave_model.setLengthEnable(
    //             getJsonValueOrDefault(wave, "length_enable", false));
    //         wave_model.setLengthTimer(
    //             getJsonValueOrDefault(wave, "length_timer", 0));
    //         wave_model.setVolume(getJsonValueOrDefault(wave, "volume", 1));

    //         std::array<uint8_t, 32> samples;
    //         size_t index = 0;
    //         for (const auto& sample : wave["samples"])
    //         {
    //             samples[index] = sample;
    //             index++;
    //             if (index == samples.size())
    //             {
    //                 break;
    //             }
    //         }
    //         wave_model.setSamples(samples);
    //     }
    //     ++instrument_index;
    // }
}

static void loadTracks(domain::Song& song, const nlohmann::json& json_root)
{
    // size_t track_index = 0;

    // for (const auto& track : json_root["song"]["tracks"])
    // {
    //     auto& track_model = song.getTrack(track_index);
    //     size_t track_element_index = 0;
    //     for (const auto& chain : track)
    //     {
    //         const auto chain_index = getJsonValue<size_t>(chain,
    //         "chain_index"); if (chain_index)
    //         {
    //             track_model.setPhraseIndex(track_element_index,
    //             *chain_index);
    //         }

    //         ++track_element_index;
    //         if (track_element_index == domain::Track::PHRASE_COUNT)
    //         {
    //             break;
    //         }
    //     }
    //     ++track_index;
    //     if (track_index == domain::Song::TRACK_COUNT)
    //     {
    //         break;
    //     }
    // }
}

bool JsonSongLoader::loadSong(
    domain::Song& song,
    const std::filesystem::path& file_path)
{
    nlohmann::json json_root;

    // Open the file
    std::ifstream file(file_path);
    if (!file.is_open())
    {
        std::cerr << "Could not open the file: " << file_path << std::endl;
        return false;
    }

    try
    {
        // Parse the file into the json object
        file >> json_root;

        // Close the file
        file.close();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        return false;
    }

    if (!json_root.contains("song"))
    {
        std::cerr << "Failed to parse json file, 'song' node is not present"
                  << std::endl;
        return false;
    }

    if (!json_root["song"].contains("phrases"))
    {
        std::cerr
            << "Failed to parse json file: 'phrases' nodes are not present"
            << std::endl;
        return false;
    }

    if (!json_root["song"].contains("instruments"))
    {
        std::cerr
            << "Failed to parse json file: 'instruments' nodes are not present"
            << std::endl;
        return false;
    }

    loadPhrases(song, json_root);
    loadInstruments(song, json_root);
    loadTracks(song, json_root);

    return true;
}

}
