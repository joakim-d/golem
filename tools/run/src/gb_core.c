#include "gb_core.h"

#include <stdlib.h>

/* Peanut-GB calls these for $FF10-$FF3F; they must be declared before the include. */
static uint8_t audio_read(uint16_t address);
static void audio_write(uint16_t address, uint8_t value);

#define ENABLE_SOUND 1
#define ENABLE_LCD 0
#include "peanut_gb.h"

struct golem_gb {
    struct gb_s gb;
    const uint8_t* rom;
    size_t size;
    golem_gb_write_fn on_write;
    void* user;
    const char* error;
};

/* Emulator running the current frame; audio_write() has no context argument. */
static golem_gb* current;

static uint8_t audio_read(uint16_t address)
{
    (void)address;
    return 0xFF; /* The driver must not read APU registers. */
}

static void audio_write(uint16_t address, uint8_t value)
{
    /* gb_reset() writes the post-boot APU state outside of a frame: not a program write. */
    if (current != NULL) {
        current->on_write(current->user, address, value);
    }
}

static uint8_t rom_read(struct gb_s* gb, const uint_fast32_t address)
{
    const golem_gb* self = gb->direct.priv;
    return address < self->size ? self->rom[address] : 0xFF;
}

static uint8_t cart_ram_read(struct gb_s* gb, const uint_fast32_t address)
{
    (void)gb;
    (void)address;
    return 0xFF;
}

static void cart_ram_write(struct gb_s* gb, const uint_fast32_t address, const uint8_t value)
{
    (void)gb;
    (void)address;
    (void)value;
}

static void on_error(struct gb_s* gb, const enum gb_error_e error, const uint16_t address)
{
    golem_gb* self = gb->direct.priv;
    (void)address;
    if (self->error != NULL) {
        return;
    }
    switch (error) {
    case GB_INVALID_OPCODE:
        self->error = "invalid opcode";
        break;
    case GB_INVALID_READ:
        self->error = "invalid read";
        break;
    case GB_INVALID_WRITE:
        self->error = "invalid write";
        break;
    default:
        self->error = "emulator error";
        break;
    }
}

golem_gb* golem_gb_create(
    const uint8_t* rom,
    size_t size,
    golem_gb_write_fn on_write,
    void* user,
    const char** error)
{
    golem_gb* self = calloc(1, sizeof *self);
    if (self == NULL) {
        *error = "out of memory";
        return NULL;
    }
    self->rom = rom;
    self->size = size;
    self->on_write = on_write;
    self->user = user;

    switch (gb_init(&self->gb, rom_read, cart_ram_read, cart_ram_write, on_error, self)) {
    case GB_INIT_NO_ERROR:
        return self;
    case GB_INIT_CARTRIDGE_UNSUPPORTED:
        *error = "unsupported cartridge type";
        break;
    case GB_INIT_INVALID_CHECKSUM:
        *error = "invalid header checksum";
        break;
    default:
        *error = "cannot initialise the emulator";
        break;
    }
    free(self);
    return NULL;
}

int golem_gb_run_frame(golem_gb* gb, const char** error)
{
    current = gb;
    gb_run_frame(&gb->gb);
    current = NULL;
    if (gb->error != NULL) {
        *error = gb->error;
        return 1;
    }
    return 0;
}

void golem_gb_destroy(golem_gb* gb)
{
    free(gb);
}
