/* Minimal C interface to the SameBoy emulator core: runs a ROM-only cartridge headless on a
 * DMG and reports every write to the APU registers ($FF10-$FF3F) with the time it happened.
 * Same shape as gb_core.h (Peanut-GB). */
#ifndef GOLEM_SAMEBOY_CORE_H
#define GOLEM_SAMEBOY_CORE_H

#include "gb_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct golem_sb golem_sb;

typedef void (*golem_sample_fn)(
    void* user,
    int16_t left,
    int16_t right);

/* Returns NULL on failure, with *error describing why. `on_write` may be NULL. */
golem_sb* golem_sb_create(
    const uint8_t* rom,
    size_t size,
    golem_gb_write_fn on_write,
    void* user,
    const char** error);

/* Runs one frame (70224 CPU cycles). Returns 0; SameBoy reports no emulation errors. */
int golem_sb_run_frame(
    golem_sb* sb,
    const char** error);

/* Starts audio output: `on_sample` gets every stereo sample, at `sample_rate` Hz, through
 * SameBoy's hardware-like high-pass filter. */
void golem_sb_set_audio(
    golem_sb* sb,
    unsigned sample_rate,
    golem_sample_fn on_sample,
    void* user);

void golem_sb_destroy(golem_sb* sb);

#ifdef __cplusplus
}
#endif

#endif
