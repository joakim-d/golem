# Driver test ROMs: the driver, the test ROM shell and one song each, checked by golem-run
# against the song's golden trace and the cycle budget, plus a size check (ctest -R driver).

# Budgets (docs/driver-steps/07-budgets.md): T-cycles per GolemPlay call, and bytes of the
# driver's ROM section ("Golem driver") and WRAM section ("Golem state").
set(GOLEM_DRIVER_MAX_PLAY_CYCLES 14000)
set(GOLEM_DRIVER_MAX_ROM_BYTES 3072)
set(GOLEM_DRIVER_MAX_WRAM_BYTES 128)

# Every driver test runs in Peanut-GB, and also in SameBoy where it is built (GCC or Clang),
# which must agree on the trace and on every call's cycles (docs/driver-steps/16-*.md).
if(GOLEM_HAS_SAMEBOY)
  set(GOLEM_DRIVER_EMULATORS both)
else()
  set(GOLEM_DRIVER_EMULATORS peanut)
endif()

find_program(RGBASM rgbasm)
find_program(RGBLINK rgblink)
find_program(RGBFIX rgbfix)

if(NOT RGBASM OR NOT RGBLINK OR NOT RGBFIX)
  message(WARNING "RGBDS (rgbasm, rgblink, rgbfix) not found: skipping the driver ROMs and tests")
  return()
endif()

set(DRIVER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(DRIVER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(DRIVER_INCLUDES apu.inc golem.inc)
set(RGBASM_FLAGS -Wall -Werror -I ${DRIVER_SOURCE_DIR} -I ${DRIVER_BINARY_DIR})

# Note tables generated from the reference player.
add_custom_command(
  OUTPUT notes.inc
  COMMAND golem-tables ${DRIVER_BINARY_DIR}/notes.inc
  DEPENDS golem-tables
  COMMENT "Generating notes.inc")

add_custom_command(
  OUTPUT golem.o
  COMMAND ${RGBASM} ${RGBASM_FLAGS} -o golem.o ${DRIVER_SOURCE_DIR}/golem.asm
  DEPENDS golem.asm ${DRIVER_INCLUDES} ${DRIVER_BINARY_DIR}/notes.inc
  COMMENT "Assembling golem.asm")

add_custom_command(
  OUTPUT test_rom.o
  COMMAND ${RGBASM} ${RGBASM_FLAGS} -o test_rom.o ${DRIVER_SOURCE_DIR}/test_rom.asm
  DEPENDS test_rom.asm ${DRIVER_INCLUDES}
  COMMENT "Assembling test_rom.asm")

set(GOLEM_DRIVER_ROMS)
set(GOLEM_DRIVER_WAVS)

# golem_driver_test(<song>): builds <song>.gb from tests/songs/<song>.gsong and adds the
# test driver.<song>, which compares its APU trace with tests/songs/<song>.trace and checks
# every GolemPlay call against the cycle budget, in GOLEM_DRIVER_EMULATORS.
function(golem_driver_test song)
  set(gsong ${PROJECT_SOURCE_DIR}/tests/songs/${song}.gsong)
  set(golden ${PROJECT_SOURCE_DIR}/tests/songs/${song}.trace)
  set(bin ${DRIVER_BINARY_DIR}/${song}.bin)
  set(stub ${DRIVER_BINARY_DIR}/${song}_song.asm)
  set(rom ${DRIVER_BINARY_DIR}/${song}.gb)
  set(map ${DRIVER_BINARY_DIR}/${song}.map)

  add_custom_command(
    OUTPUT ${bin}
    COMMAND golem-encode ${gsong} ${bin} --base 0x4000
    DEPENDS golem-encode ${gsong}
    COMMENT "Encoding ${song}.gsong")

  file(WRITE ${stub} "SECTION \"Song\", ROMX[$4000]\nSong::\n\tINCBIN \"${bin}\"\n")
  add_custom_command(
    OUTPUT ${song}_song.o
    COMMAND ${RGBASM} ${RGBASM_FLAGS} -o ${song}_song.o ${stub}
    DEPENDS ${bin}
    COMMENT "Assembling ${song} song")

  add_custom_command(
    OUTPUT ${rom}
    BYPRODUCTS ${map}
    COMMAND ${RGBLINK} -o ${rom} -m ${map} test_rom.o golem.o ${song}_song.o
    COMMAND ${RGBFIX} -v -p 0xFF -t GOLEMTEST ${rom}
    DEPENDS test_rom.o golem.o ${song}_song.o
    COMMENT "Linking ${song}.gb")

  set(GOLEM_DRIVER_ROMS ${GOLEM_DRIVER_ROMS} ${rom} PARENT_SCOPE)

  # For listening (target driver-wavs): as many frames as the golden trace covers.
  if(GOLEM_HAS_SAMEBOY)
    file(STRINGS ${golden} header LIMIT_COUNT 1)
    string(REGEX MATCH "frames=([0-9]+)" _ "${header}")
    set(wav ${DRIVER_BINARY_DIR}/${song}.wav)
    add_custom_command(
      OUTPUT ${wav}
      COMMAND golem-wav ${rom} ${wav} --frames ${CMAKE_MATCH_1}
      DEPENDS golem-wav ${rom}
      COMMENT "Rendering ${song}.wav")
    set(GOLEM_DRIVER_WAVS ${GOLEM_DRIVER_WAVS} ${wav} PARENT_SCOPE)
  endif()
  add_test(NAME driver.${song}
    COMMAND golem-run ${rom} --expect ${golden} --cycles
      --max-cycles ${GOLEM_DRIVER_MAX_PLAY_CYCLES} --emulator ${GOLEM_DRIVER_EMULATORS})
endfunction()

# One test per step of docs/driver-steps/: every song listed here must match its trace.
golem_driver_test(silence) # Step 0
golem_driver_test(scale) # Step 1
golem_driver_test(pulses) # Step 2
golem_driver_test(wave) # Step 3
golem_driver_test(noise) # Step 4
golem_driver_test(minimal) # Step 4
golem_driver_test(instruments) # Step 4
golem_driver_test(flow) # Step 5
golem_driver_test(jumps) # Step 5
golem_driver_test(registers) # Step 6
golem_driver_test(effects) # Step 6
golem_driver_test(cut) # Step 8
golem_driver_test(delay) # Step 9
golem_driver_test(slide) # Step 10
golem_driver_test(arpeggio) # Step 12
golem_driver_test(portamento) # Step 13
golem_driver_test(tone) # Step 14
golem_driver_test(vibrato) # Step 15

add_custom_target(driver-roms ALL DEPENDS ${GOLEM_DRIVER_ROMS})

# The editor's player ROM template: the test ROM shell and the driver, with an empty song slot
# at $4000 that the editor fills with an encoded song (docs/editor-steps/01-live-playback.md).
# It is compiled into golem_run as a byte array (GOLEM_PLAYER_ROM_SOURCE).
set(player_stub ${DRIVER_BINARY_DIR}/player_song.asm)
file(WRITE ${player_stub} "SECTION \"Song\", ROMX[$4000]\nSong::\n\tds 1 ; Filled in by the editor.\n")
add_custom_command(
  OUTPUT player_song.o
  COMMAND ${RGBASM} ${RGBASM_FLAGS} -o player_song.o ${player_stub}
  COMMENT "Assembling the player's song slot")
add_custom_command(
  OUTPUT ${DRIVER_BINARY_DIR}/player.gb
  COMMAND ${RGBLINK} -o ${DRIVER_BINARY_DIR}/player.gb test_rom.o golem.o player_song.o
  COMMAND ${RGBFIX} -v -p 0xFF -t GOLEM ${DRIVER_BINARY_DIR}/player.gb
  DEPENDS test_rom.o golem.o player_song.o
  COMMENT "Linking player.gb")
add_custom_command(
  OUTPUT ${DRIVER_BINARY_DIR}/player_rom.cpp
  COMMAND ${CMAKE_COMMAND} -DINPUT=${DRIVER_BINARY_DIR}/player.gb
    -DOUTPUT=${DRIVER_BINARY_DIR}/player_rom.cpp -DSYMBOL=kPlayerRom
    -P ${PROJECT_SOURCE_DIR}/cmake/EmbedFile.cmake
  DEPENDS ${DRIVER_BINARY_DIR}/player.gb ${PROJECT_SOURCE_DIR}/cmake/EmbedFile.cmake
  COMMENT "Embedding player.gb")
add_custom_target(player-rom DEPENDS ${DRIVER_BINARY_DIR}/player_rom.cpp)
set(GOLEM_PLAYER_ROM_SOURCE ${DRIVER_BINARY_DIR}/player_rom.cpp PARENT_SCOPE)
if(GOLEM_HAS_SAMEBOY)
  # Not part of the default build: render on demand to listen to the driver songs.
  add_custom_target(driver-wavs DEPENDS ${GOLEM_DRIVER_WAVS})
endif()

# The driver sections are the same in every test ROM: check them in one map file.
add_test(NAME driver.size
  COMMAND ${CMAKE_COMMAND} -DMAP=${DRIVER_BINARY_DIR}/silence.map
    -DMAX_ROM=${GOLEM_DRIVER_MAX_ROM_BYTES} -DMAX_WRAM=${GOLEM_DRIVER_MAX_WRAM_BYTES}
    -P ${DRIVER_SOURCE_DIR}/check_size.cmake)
