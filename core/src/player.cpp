#include "golem/player.h"

#include "golem/notes.h"

#include <string>

namespace golem {

namespace {

    constexpr std::uint8_t kTrigger = 0x80;
    constexpr std::uint8_t kLengthEnable = 0x40;
    constexpr std::uint8_t kNoiseShortLfsr = 0x08;
    constexpr unsigned kMaxTicksPerRow = 256;

    enum Effect : std::uint8_t {
        kSetMasterVolume = 0x5,
        kCallRoutine = 0x6,
        kSetPanning = 0x8,
        kChangeTimbre = 0x9,
        kPositionJump = 0xB,
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

    bool is_supported(std::uint8_t effect)
    {
        switch (effect) {
        case kSetMasterVolume:
        case kCallRoutine:
        case kSetPanning:
        case kChangeTimbre:
        case kPositionJump:
        case kSetVolume:
        case kPatternBreak:
        case kNoteCut:
        case kSetTempo:
            return true;
        default:
            return false;
        }
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
        if (tick_ == 0) {
            row_length_ = ticks_per_row_;
            for (auto& channel : channels_) {
                channel.cut_tick.reset();
            }
            play_row();
        } else {
            for (std::size_t channel = 0; channel < kChannels; ++channel) {
                if (channels_[channel].cut_tick == tick_) {
                    channels_[channel].cut_tick.reset();
                    cut(channel);
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
        if (!is_empty_effect(cell) && !is_supported(cell.effect)) {
            throw UnsupportedEffect(
                "order "
                + std::to_string(order_)
                + " row "
                + std::to_string(row_)
                + " channel "
                + std::to_string(channel + 1)
                + ": effect "
                + std::to_string(cell.effect)
                + " is not implemented");
        }
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
        if (channel == kPulse1) {
            write(reg::NR10, pulse.nr10());
        }
        write(base + 1, overrides.timbre.value_or(pulse.nrx1()));
        write(base + 2, overrides.volume.value_or(pulse.nrx2()));
        write(base + 3, low(state.period));
        write(base + 4, kTrigger | length | high(state.period));
    } else if (channel == kWave) {
        const auto& wave = song_.wave_instruments[instrument];
        state.period = note_period(note);
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
        write(reg::NR42, overrides.volume.value_or(noise.nr42()));
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
    const Channel& state = channels_[channel];
    const std::uint8_t length = length_enable_bit(channel);
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
