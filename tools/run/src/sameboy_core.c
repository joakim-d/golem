#include "sameboy_core.h"

#include "Core/gb.h"

#include <stdlib.h>

#define BOOT_ROM_SIZE 256
#define TICKS_PER_CYCLE 2 /* GB_run counts 8 MHz ticks: 2 per DMG CPU cycle. */
#define CYCLES_PER_FRAME 70224

struct golem_sb {
    GB_gameboy_t* gb;
    uint64_t ticks; /* Time before the instruction being run, in 8 MHz ticks. */
    golem_gb_write_fn on_write;
    void* user;
};

static bool write_hook(GB_gameboy_t* gb, uint16_t address, uint8_t value)
{
    golem_sb* self = GB_get_user_data(gb);
    if (address >= 0xFF10 && address <= 0xFF3F) {
        self->on_write(
            self->user, address, value, (uint16_t)(self->ticks / TICKS_PER_CYCLE));
    }
    return true;
}

/* A stand-in for the DMG boot ROM: turns the LCD on as the real one leaves it (so VBlank
 * interrupts happen), then unmaps itself just as execution reaches $0100. */
static void make_boot_rom(uint8_t* boot)
{
    for (size_t i = 0; i < BOOT_ROM_SIZE; ++i) {
        boot[i] = 0x00; /* nop */
    }
    boot[0x00] = 0x3E; /* ld a, $91 */
    boot[0x01] = 0x91;
    boot[0x02] = 0xE0; /* ldh [$FF40], a: LCDC */
    boot[0x03] = 0x40;
    boot[0xFC] = 0x3E; /* ld a, $01 */
    boot[0xFD] = 0x01;
    boot[0xFE] = 0xE0; /* ldh [$FF50], a: unmap the boot ROM; next is $0100 */
    boot[0xFF] = 0x50;
}

golem_sb* golem_sb_create(
    const uint8_t* rom,
    size_t size,
    golem_gb_write_fn on_write,
    void* user,
    const char** error)
{
    golem_sb* self = calloc(1, sizeof *self);
    if (self == NULL) {
        *error = "out of memory";
        return NULL;
    }
    self->on_write = on_write;
    self->user = user;
    self->gb = GB_init(GB_alloc(), GB_MODEL_DMG_B);
    GB_set_user_data(self->gb, self);

    uint8_t boot[BOOT_ROM_SIZE];
    make_boot_rom(boot);
    GB_load_boot_rom_from_buffer(self->gb, boot, sizeof boot);
    GB_load_rom_from_buffer(self->gb, rom, size);
    GB_set_rendering_disabled(self->gb, true); /* Headless: no pixel buffer. */
    GB_set_turbo_mode(self->gb, true, true); /* As fast as possible, not paced to real time. */
    GB_set_write_memory_callback(self->gb, write_hook);
    return self;
}

int golem_sb_run_frame(golem_sb* sb, const char** error)
{
    (void)error;
    const uint64_t end = sb->ticks + (uint64_t)CYCLES_PER_FRAME * TICKS_PER_CYCLE;
    while (sb->ticks < end) {
        sb->ticks += GB_run(sb->gb);
    }
    return 0;
}

void golem_sb_destroy(golem_sb* sb)
{
    GB_free(sb->gb);
    GB_dealloc(sb->gb);
    free(sb);
}
