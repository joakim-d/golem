; Golem sound driver: public interface. Behaviour: docs/driver-contract.md.
;
; GolemInit::
;   Starts playing a song from its first row.
;   In:  hl = address of the song (binary song format, encoded for that address; the song
;        must stay mapped there while the driver runs)
;   Writes the frame 0 registers (see "Frames" in the contract).
;   Clobbers af, bc, de, hl.
;
; GolemPlay::
;   Plays one frame. Call once per frame (VBlank), after GolemInit.
;   Clobbers af, bc, de, hl.
;
; The driver keeps its state in WRAM0 and never reads APU registers.
