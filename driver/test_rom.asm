; Driver test ROM: plays the song linked at `Song` and marks every frame for golem-run,
; which logs the APU writes between markers (tools/run/include/golem/rom_runner.h).

INCLUDE "golem.inc"

DEF FRAME_MARKER EQU $FF15 ; Unused APU address; any write starts a frame.
DEF rIF EQU $FF0F
DEF rIE EQU $FFFF
DEF IE_VBLANK EQU %00000001

SECTION "VBlank interrupt", ROM0[$40]
	reti

SECTION "Header", ROM0[$100]
	nop
	jp Start
	ds $150 - @, 0 ; Cartridge header, filled in by rgbfix

SECTION "Test ROM", ROM0
Start:
	di
	ld sp, $E000
	ldh [FRAME_MARKER], a ; Frame 0
	ld hl, Song
	call GolemInit

	ld a, IE_VBLANK
	ldh [rIE], a
	xor a
	ldh [rIF], a
	ei
.frame
	halt
	ldh [FRAME_MARKER], a ; Frames 1, 2, ...
	call GolemPlay
	jr .frame
