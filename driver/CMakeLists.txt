# Driver test ROMs: the driver, the test ROM shell and one song each, checked by golem-run
# against the song's golden trace and the cycle budget, plus a size check (ctest -R driver).

# Budgets (docs/driver-steps/07-budgets.md): T-cycles per GolemPlay call, and bytes of the
# driver's ROM section ("Golem driver") and WRAM section ("Golem state").
set(GOLEM_DRIVER_MAX_PLAY_CYCLES 14000)
set(GOLEM_DRIVER_MAX_ROM_BYTES 2048)
set(GOLEM_DRIVER_MAX_WRAM_BYTES 64)

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

# golem_driver_test(<song>): builds <song>.gb from tests/songs/<song>.gsong and adds the
# test driver.<song>, which compares its APU trace with tests/songs/<song>.trace and checks
# every GolemPlay call against the cycle budget.
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
  add_test(NAME driver.${song}
    COMMAND golem-run ${rom} --expect ${golden} --cycles
      --max-cycles ${GOLEM_DRIVER_MAX_PLAY_CYCLES})
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

add_custom_target(driver-roms ALL DEPENDS ${GOLEM_DRIVER_ROMS})

# The driver sections are the same in every test ROM: check them in one map file.
add_test(NAME driver.size
  COMMAND ${CMAKE_COMMAND} -DMAP=${DRIVER_BINARY_DIR}/silence.map
    -DMAX_ROM=${GOLEM_DRIVER_MAX_ROM_BYTES} -DMAX_WRAM=${GOLEM_DRIVER_MAX_WRAM_BYTES}
    -P ${DRIVER_SOURCE_DIR}/check_size.cmake)
