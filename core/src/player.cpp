#include "golem/player.h"

#include "golem/notes.h"

#include <algorithm>

#include <string>

namespace golem {

namespace {

    constexpr std::uint8_t kTrigger = 0x80;
    constexpr std::uint8_t kLengthEnable = 0x40;
    constexpr std::uint8_t kNoiseShortLfsr = 0x08;
    constexpr unsigned kMaxTicksPerRow = 256;

    enum Effect : std::uint8_t {
        kArpeggio = 0x0,
        kPortamentoUp = 0x1,
        kPortamentoDown = 0x2,
        kTonePortamento = 0x3,
        kVibrato = 0x4,
        kSetMasterVolume = 0x5,
        kCallRoutine = 0x6,
        kNoteDelay = 0x7,
        kSetPanning = 0x8,
        kChangeTimbre = 0x9,
        kPositionJump = 0xB,
        kVolumeSlide = 0xA,
        kSetVolume = 0xC,
        kPatternBreak = 0xD,
        kNoteCut = 0xE,
        kSetTempo = 0xF,
    };

    constexpr std::size_t kPulse1 = 0;
    constexpr std::size_t kPulse2 = 1;
    constexpr std::size_t kWave = 2;
    constexpr std::size_t kNoise = 3;

    unsigned ticks(std::uint8_t ticks_per_row)
    {
        return ticks_per_row == 0 ? kMaxTicksPerRow : ticks_per_row;
    }

    bool is_empty_effect(const Cell& cell)
    {
        return cell.effect == 0 && cell.param == 0;
    }

    std::uint8_t low(std::uint16_t period)
    {
        return static_cast<std::uint8_t>(period & 0xFF);
    }

    std::uint8_t high(std::uint16_t period)
    {
        return static_cast<std::uint8_t>(period >> 8 & 0x07);
    }

    // NR32 output level from the x nibble of effect C.
    std::uint8_t wave_volume(std::uint8_t param)
    {
        return static_cast<std::uint8_t>((param >> 4 & 0x03) << 5);
    }

    // Register base of a pulse channel: NR10 or NR21 - 1, so that base + n is NRxn.
    std::uint16_t pulse_base(std::size_t channel)
    {
        return channel == kPulse1 ? reg::NR10 : static_cast<std::uint16_t>(reg::NR21 - 1);
    }

} // namespace

Player::Player(Song song)
    : song_(std::move(song))
    , ticks_per_row_(ticks(song_.ticks_per_row))
{
    validate_song(song_);
}

std::vector<ApuWrite> Player::step()
{
    writes_.clear();
    if (frame_ == 0) {
        init();
    } else {
        position_ = Position {order_, row_, tick_};
        if (tick_ == 0) {
            row_length_ = ticks_per_row_;
            for (auto& channel : channels_) {
                channel.cut_tick.reset();
                channel.delay_tick.reset();
                channel.slide.reset();
                channel.arpeggio.reset();
                channel.portamento.reset();
                channel.tone_portamento.reset();
                channel.vibrato.reset();
            }
            play_row();
        } else {
            // Timed effects due on this tick (E, 7), slide steps (A), arpeggio steps (0),
            // portamento steps (1, 2, 3) and vibrato steps (4), in channel order.
            // A channel has at most one: a cell holds a single effect.
            for (std::size_t channel = 0; channel < kChannels; ++channel) {
                Channel& state = channels_[channel];
                if (state.cut_tick == tick_) {
                    state.cut_tick.reset();
                    cut(channel);
                } else if (state.delay_tick == tick_) {
                    state.delay_tick.reset();
                    trigger(channel, state.delay_note, {});
                } else if (state.slide) {
                    slide_volume(channel);
                } else if (state.arpeggio) {
                    arpeggio_step(channel);
                } else if (state.portamento) {
                    portamento_step(channel);
                } else if (state.tone_portamento) {
                    tone_portamento_step(channel);
                } else if (state.vibrato) {
                    vibrato_step(channel);
                }
            }
        }
        if (++tick_ == row_length_) {
            tick_ = 0;
            if (jump_order_ || break_row_) {
                order_ = jump_order_ ? *jump_order_ : order_ + 1;
                row_ = break_row_ ? *break_row_ : 0;
                if (order_ >= song_.orders.size()) {
                    order_ = 0;
                }
                if (row_ >= kRowsPerPattern) {
                    row_ = 0;
                }
                jump_order_.reset();
                break_row_.reset();
            } else if (++row_ == kRowsPerPattern) {
                row_ = 0;
                order_ = (order_ + 1) % song_.orders.size();
            }
        }
    }
    ++frame_;
    return std::move(writes_);
}

bool Player::Position::operator==(const Position& other) const
{
    return order == other.order && row == other.row && tick == other.tick;
}

Player::Position Player::position() const
{
    return position_;
}

void Player::init()
{
    write(reg::NR52, 0x80);
    write(reg::NR50, 0x77);
    write(reg::NR51, 0xFF);
}

void Player::play_row()
{
    for (std::size_t channel = 0; channel < kChannels; ++channel) {
        const auto& pattern = song_.patterns[song_.orders[order_][channel]];
        const Cell& cell = pattern[row_];
        restore_pitch(channel, cell);
        play_cell(channel, cell);
    }
}

void Player::play_cell(
    std::size_t channel,
    const Cell& cell)
{
    if (cell.instrument != 0) {
        channels_[channel].instrument = cell.instrument;
    }
    const bool has_effect = !is_empty_effect(cell);
    Channel& state = channels_[channel];
    if (cell.note != kNoteNone
        && cell.effect == kTonePortamento
        && channel != kNoise
        && state.note != kNoteNone) {
        // 3 with a note: no trigger. The note becomes the channel's note and the target its
        // period slides toward.
        state.note = cell.note;
        state.target = note_period(cell.note);
        state.tone_portamento = cell.param;
        return;
    }
    if (cell.note != kNoteNone && cell.effect == kNoteDelay && cell.param != 0) {
        // 7: the trigger moves to tick xx of the row, or is dropped past the row.
        if (cell.param < row_length_) {
            channels_[channel].delay_tick = cell.param;
            channels_[channel].delay_note = cell.note;
        }
        return;
    }
    if (cell.note != kNoteNone) {
        Overrides overrides;
        if (has_effect && cell.effect == kChangeTimbre) {
            overrides.timbre = cell.param;
        } else if (has_effect && cell.effect == kSetVolume) {
            overrides.volume = cell.param;
        }
        trigger(channel, cell.note, overrides);
        if (overrides.timbre || overrides.volume) {
            return;
        }
    }
    if (has_effect) {
        apply_effect(channel, cell);
    }
}

void Player::trigger(
    std::size_t channel,
    std::uint8_t note,
    const Overrides& overrides)
{
    Channel& state = channels_[channel];
    const std::size_t instrument = state.instrument - 1;
    const std::uint8_t length = length_enable_bit(channel);

    if (channel == kPulse1 || channel == kPulse2) {
        const auto& pulse = song_.pulse_instruments[instrument];
        const auto base = pulse_base(channel);
        state.period = note_period(note);
        state.note = note;
        state.pitch = state.period;
        state.target.reset();
        if (channel == kPulse1) {
            write(reg::NR10, pulse.nr10());
        }
        write(base + 1, overrides.timbre.value_or(pulse.nrx1()));
        const std::uint8_t nrx2 = overrides.volume.value_or(pulse.nrx2());
        state.volume = nrx2 >> 4;
        write(base + 2, nrx2);
        write(base + 3, low(state.period));
        write(base + 4, kTrigger | length | high(state.period));
    } else if (channel == kWave) {
        const auto& wave = song_.wave_instruments[instrument];
        state.period = note_period(note);
        state.note = note;
        state.pitch = state.period;
        state.target.reset();
        load_wave(overrides.timbre ? (*overrides.timbre & 0x0F) : wave.wave_index());
        write(reg::NR30, 0x80);
        write(reg::NR31, wave.nr31());
        write(reg::NR32, overrides.volume ? wave_volume(*overrides.volume) : wave.nr32());
        write(reg::NR33, low(state.period));
        write(reg::NR34, kTrigger | length | high(state.period));
    } else {
        const auto& noise = song_.noise_instruments[instrument];
        const bool short_lfsr = overrides.timbre ? *overrides.timbre != 0 : noise.short_lfsr();
        state.noise = noise_nr43(note);
        write(reg::NR41, noise.nr41());
        const std::uint8_t nr42 = overrides.volume.value_or(noise.nr42());
        state.volume = nr42 >> 4;
        write(reg::NR42, nr42);
        write(reg::NR43, state.noise | (short_lfsr ? kNoiseShortLfsr : 0));
        write(reg::NR44, kTrigger | length);
    }
}

void Player::apply_effect(
    std::size_t channel,
    const Cell& cell)
{
    const Channel& state = channels_[channel];
    const std::uint8_t length = length_enable_bit(channel);
    switch (cell.effect) {
    case kSetMasterVolume:
        write(reg::NR50, cell.param);
        break;
    case kSetPanning:
        write(reg::NR51, cell.param);
        break;
    case kChangeTimbre:
        if (channel == kPulse1 || channel == kPulse2) {
            write(pulse_base(channel) + 1, cell.param);
        } else if (channel == kWave) {
            const std::uint8_t index = cell.param & 0x0F;
            if (loaded_wave_ != index) {
                load_wave(index);
                write(reg::NR30, 0x80);
                write(reg::NR34, kTrigger | length | high(state.period));
            }
        } else {
            write(reg::NR43, state.noise | (cell.param != 0 ? kNoiseShortLfsr : 0));
        }
        break;
    case kSetVolume:
        set_volume(channel, cell.param);
        break;
    case kArpeggio:
        if (channel != kNoise) { // 0 has no effect on the noise channel.
            channels_[channel].arpeggio = cell.param;
        }
        break;
    case kVibrato:
        if (channel != kNoise) { // 4 has no effect on the noise channel.
            channels_[channel].vibrato = cell.param;
        }
        break;
    case kTonePortamento:
        if (channel != kNoise) { // 3 has no effect on the noise channel.
            channels_[channel].tone_portamento = cell.param;
        }
        break;
    case kPortamentoUp:
    case kPortamentoDown:
        if (channel != kNoise) { // 1 and 2 have no effect on the noise channel.
            channels_[channel].portamento =
                cell.effect == kPortamentoUp ? int {cell.param} : -int {cell.param};
        }
        break;
    case kVolumeSlide:
        if (channel != kWave) { // A has no effect on the wave channel.
            channels_[channel].slide = cell.param;
        }
        break;
    case kNoteCut:
        if (cell.param == 0) {
            cut(channel);
        } else if (cell.param < row_length_) {
            channels_[channel].cut_tick = cell.param;
        }
        break;
    case kPositionJump:
        jump_order_ = cell.param;
        break;
    case kPatternBreak:
        break_row_ = cell.param;
        break;
    case kSetTempo:
        ticks_per_row_ = ticks(cell.param);
        break;
    default: // kCallRoutine
        break;
    }
}

void Player::set_volume(
    std::size_t channel,
    std::uint8_t param)
{
    Channel& state = channels_[channel];
    const std::uint8_t length = length_enable_bit(channel);
    state.volume = param >> 4;
    if (channel == kPulse1 || channel == kPulse2) {
        write(pulse_base(channel) + 2, param);
        write(pulse_base(channel) + 4, kTrigger | length | high(state.period));
    } else if (channel == kWave) {
        write(reg::NR32, wave_volume(param));
    } else {
        write(reg::NR42, param);
        write(reg::NR44, kTrigger | length);
    }
}

void Player::cut(std::size_t channel)
{
    set_volume(channel, 0x00);
}

void Player::slide_volume(std::size_t channel)
{
    Channel& state = channels_[channel];
    const unsigned up = *state.slide >> 4;
    const unsigned down = *state.slide & 0x0F;
    const unsigned volume = up != 0 ? std::min(state.volume + up, 15u)
                                    : (state.volume > down ? state.volume - down : 0u);
    if (volume == state.volume) {
        return;
    }
    state.volume = static_cast<std::uint8_t>(volume);
    const std::uint8_t length = length_enable_bit(channel);
    const auto nrx2 = static_cast<std::uint8_t>(volume << 4); // Envelope pace 0.
    if (channel == kPulse1 || channel == kPulse2) {
        write(pulse_base(channel) + 2, nrx2);
        write(pulse_base(channel) + 4, kTrigger | length | high(state.period));
    } else {
        write(reg::NR42, nrx2);
        write(reg::NR44, kTrigger | length);
    }
}

void Player::arpeggio_step(std::size_t channel)
{
    const Channel& state = channels_[channel];
    if (state.note == kNoteNone) {
        return;
    }
    const unsigned step = tick_ % 3;
    std::uint16_t period = state.period; // The base step: the channel's (maybe slid) period.
    if (step != 0) {
        const unsigned offset = step == 1 ? *state.arpeggio >> 4 : *state.arpeggio & 0x0F;
        period = note_period(
            static_cast<std::uint8_t>(std::min(state.note + offset, unsigned {kLastNote})));
    }
    if (period != state.pitch) {
        write_pitch(channel, period);
    }
}

void Player::portamento_step(std::size_t channel)
{
    Channel& state = channels_[channel];
    if (state.note == kNoteNone) {
        return;
    }
    const int lowest = note_period(kFirstNote);
    const int highest = note_period(kLastNote);
    const auto period = static_cast<std::uint16_t>(
        std::clamp(int {state.period} + *state.portamento, lowest, highest));
    if (period != state.period) {
        state.period = period;
        write_pitch(channel, period);
    }
}

void Player::tone_portamento_step(std::size_t channel)
{
    Channel& state = channels_[channel];
    if (!state.target || *state.tone_portamento == 0) {
        return;
    }
    const int period = state.period;
    const int target = *state.target;
    const int step = *state.tone_portamento;
    const int moved =
        period < target ? std::min(period + step, target) : std::max(period - step, target);
    if (moved != period) {
        state.period = static_cast<std::uint16_t>(moved);
        write_pitch(channel, state.period);
    }
}

bool Player::triggers_on_row_tick(
    std::size_t channel,
    const Cell& cell) const
{
    if (cell.note == kNoteNone) {
        return false;
    }
    if (cell.effect == kNoteDelay && cell.param != 0) {
        return false;
    }
    if (cell.effect == kTonePortamento
        && channel != kNoise
        && channels_[channel].note != kNoteNone) {
        return false;
    }
    return true;
}

void Player::vibrato_step(std::size_t channel)
{
    const Channel& state = channels_[channel];
    if (state.note == kNoteNone) {
        return;
    }
    const unsigned speed = std::max(*state.vibrato >> 4, 1);
    const int depth = *state.vibrato & 0x0F;
    const bool up = ((tick_ - 1) / speed) % 2 == 0;
    const int lowest = note_period(kFirstNote);
    const int highest = note_period(kLastNote);
    const auto period = static_cast<std::uint16_t>(
        std::clamp(int {state.period} + (up ? depth : -depth), lowest, highest));
    if (period != state.pitch) {
        write_pitch(channel, period);
    }
}

void Player::restore_pitch(
    std::size_t channel,
    const Cell& cell)
{
    const Channel& state = channels_[channel];
    if (channel == kNoise || state.pitch == state.period) {
        return;
    }
    if (triggers_on_row_tick(channel, cell)) {
        return; // The trigger writes the new note's pitch.
    }
    write_pitch(channel, state.period);
}

void Player::write_pitch(
    std::size_t channel,
    std::uint16_t period)
{
    channels_[channel].pitch = period;
    const std::uint8_t length = length_enable_bit(channel);
    if (channel == kWave) {
        write(reg::NR33, low(period));
        write(reg::NR34, length | high(period));
    } else {
        write(pulse_base(channel) + 3, low(period));
        write(pulse_base(channel) + 4, length | high(period));
    }
}

void Player::load_wave(std::uint8_t index)
{
    if (loaded_wave_ == index) {
        return;
    }
    write(reg::NR30, 0x00);
    for (std::size_t i = 0; i < kWaveBytes; ++i) {
        write(static_cast<std::uint16_t>(reg::WAVE_RAM + i), song_.waves[index][i]);
    }
    loaded_wave_ = index;
}

std::uint8_t Player::length_enable_bit(std::size_t channel) const
{
    const std::size_t instrument = channels_[channel].instrument - 1;
    bool enabled;
    if (channel == kWave) {
        enabled = song_.wave_instruments[instrument].length_enable();
    } else if (channel == kNoise) {
        enabled = song_.noise_instruments[instrument].length_enable();
    } else {
        enabled = song_.pulse_instruments[instrument].length_enable();
    }
    return enabled ? kLengthEnable : 0;
}

void Player::write(
    std::uint16_t address,
    std::uint8_t value)
{
    writes_.push_back({address, value});
}

std::vector<std::vector<ApuWrite>> render(
    const Song& song,
    std::size_t frames)
{
    Player player(song);
    std::vector<std::vector<ApuWrite>> result;
    result.reserve(frames);
    for (std::size_t frame = 0; frame < frames; ++frame) {
        result.push_back(player.step());
    }
    return result;
}

} // namespace golem
