#include <golem/infrastructure/exporter/CProjectExporter.h>

#include <golem/domain/Project.h>
#include <golem/domain/Song.h>

#include <fstream>
#include <iomanip>

namespace infrastructure
{

constexpr uint8_t MAX_SILENCE_VALUE
    = 63; // The maximum silence duration we can insert

static void printValue(
    uint8_t value,
    std::ofstream& stream,
    const char* comment = {},
    bool add_comma = true)
{
    stream << "0x" << std::hex << std::setw(2) << std::setfill('0')
           << static_cast<unsigned>(value);

    if (add_comma)
    {
        stream << ",";
    }

    if (comment)
    {
        stream << " // " << comment;
    }

    stream << std::endl;
}

// static void exportTrack(const domain::Track& track, std::ofstream& stream)
// {
//     const auto note_count = track.noteCount();

//     unsigned silence_duration = 0;
//     for (unsigned i = 0; i < note_count; ++i)
//     {
//         const auto& note = track.note(i);
//         if (!note.has_value())
//         {
//             silence_duration++;
//             continue;
//         }

//         if (silence_duration > 0)
//         {
//             while (silence_duration > MAX_SILENCE_VALUE)
//             {
//                 printValue(MAX_SILENCE_VALUE, stream, "Silence");
//                 silence_duration -= MAX_SILENCE_VALUE;
//             }
//             printValue(silence_duration, stream, "Silence");
//         }

//         silence_duration = 0;

//         uint8_t high_note_value = 0x80 | note->duration() << 2
//             | static_cast<uint8_t>(note->frequency()) >> 4;

//         uint8_t low_note_value
//             = (static_cast<uint8_t>(note->frequency()) & 0x0F) << 4
//             | note->instrumentIndex() & 0xF;

//         printValue(high_note_value, stream, "High note value");
//         printValue(low_note_value, stream, "Low note value");
//     }

//     printValue(0, stream, "End of track");
// }

// static void exportSong(const domain::Song& song, std::ofstream& stream)
// {
//     printValue(song.ticksPerNote(), stream, "Ticks/Note");
//     song.visitTracks([&stream](const domain::Track& track)
//                      { exportTrack(track, stream); });
// }

void CProjectExporter::exportProject(
    const domain::Project& project,
    const std::string& filepath)
{
    std::ofstream stream;
    stream.open(filepath);

    if (!stream.is_open())
    {
        return;
    }

    stream << "#ifndef __GOLEM_PROJECT__" << std::endl
           << "#define __GOLEM_PROJECT__" << std::endl;

    stream << "uint8_t PROJECT[] = {" << std::endl;

    printValue(project.getSongs().size(), stream, "Song count (8bits)");
    printValue(
        project.getInstruments().size(), stream, "Instrument count (8bits)");

    for (const auto& song : project.getSongs())
    {
        // exportSong(song, stream);
    }

    printValue(0, stream, "End byte");

    stream << "};" << std::endl;

    stream << "#endif //__GOLEM_PROJECT__" << std::endl;
}

}
