/* Minimal C interface to the Peanut-GB emulator: runs a ROM-only cartridge headless and
 * reports every write to the APU registers ($FF10-$FF3F). */
#ifndef GOLEM_GB_CORE_H
#define GOLEM_GB_CORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*golem_gb_write_fn)(
    void* user,
    uint16_t address,
    uint8_t value);

typedef struct golem_gb golem_gb;

/* Returns NULL on failure, with *error describing why. `rom` must outlive the emulator. */
golem_gb* golem_gb_create(
    const uint8_t* rom,
    size_t size,
    golem_gb_write_fn on_write,
    void* user,
    const char** error);

/* Runs one emulator frame. Returns 0, or nonzero with *error set if the emulator reported
 * an error (invalid opcode, read or write). Not reentrant: one frame runs at a time. */
int golem_gb_run_frame(
    golem_gb* gb,
    const char** error);

void golem_gb_destroy(golem_gb* gb);

#ifdef __cplusplus
}
#endif

#endif
