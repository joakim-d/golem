SECTION "Header", ROM0[$100]
	jp Main
	ds $150 - @, 0 ; Cartridge header, filled in by rgbfix

SECTION "Main", ROM0
Main:
	di
	jr @
