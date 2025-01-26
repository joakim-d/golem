#pragma once

#include <core/Channel.h>
#include <model/Note.h>
#include <model/instruments/WaveInstrument.h>

#include <memory>
#include <optional>

class IAudioProcessingUnit;

namespace model {
class Instrument;
class PulseInstrument;
class Song;
class WaveInstrument;
}

namespace core {

class NotePlayer {
public:
    NotePlayer(
        std::shared_ptr<model::Song> song_model,
        std::shared_ptr<IAudioProcessingUnit> audio_processing_unit);

    void play(
        Channel channel,
        size_t instrument_index,
        model::NoteFrequency note);

    void stop(Channel channel);

private:
    void setChannel1Instrument(
        const model::PulseInstrument& pulse,
        model::NoteFrequency note);

    void setChannel2Instrument(
        const model::PulseInstrument& pulse,
        model::NoteFrequency note);

    void setChannel3Instrument(
        const model::WaveInstrument& wave,
        model::NoteFrequency note);

    void stopChannel1();
    void stopChannel2();
    void stopChannel3();

    std::shared_ptr<model::Song> m_song_model;
    std::shared_ptr<IAudioProcessingUnit> m_audio_processing_unit;
    model::WaveInstrument m_wave;
};

}
