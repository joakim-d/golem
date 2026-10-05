find_program(RGBASM rgbasm)
find_program(RGBLINK rgblink)
find_program(RGBFIX rgbfix)

if(NOT RGBASM OR NOT RGBLINK OR NOT RGBFIX)
  message(WARNING "RGBDS (rgbasm, rgblink, rgbfix) not found: skipping the GameBoy ROM")
  return()
endif()

set(GOLEM_ROM ${CMAKE_CURRENT_BINARY_DIR}/golem.gb)

add_custom_command(
  OUTPUT main.o
  COMMAND ${RGBASM} -o main.o ${CMAKE_CURRENT_SOURCE_DIR}/main.asm
  DEPENDS main.asm
  COMMENT "Assembling main.asm")

add_custom_command(
  OUTPUT ${GOLEM_ROM}
  COMMAND ${RGBLINK} -o ${GOLEM_ROM} main.o
  COMMAND ${RGBFIX} -v -p 0xFF -t GOLEM ${GOLEM_ROM}
  DEPENDS main.o
  COMMENT "Linking golem.gb")

add_custom_target(rom ALL DEPENDS ${GOLEM_ROM})
