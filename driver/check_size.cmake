# Checks the driver's ROM and WRAM sections in an rgblink map file against their limits.
#   cmake -DMAP=<file.map> -DMAX_ROM=<bytes> -DMAX_WRAM=<bytes> -P check_size.cmake

file(READ "${MAP}" map)

function(section_size name out)
  string(REGEX MATCH "\\(\\$([0-9a-fA-F]+) bytes?\\) \\[\"${name}\"\\]" match "${map}")
  if(NOT match)
    message(FATAL_ERROR "section \"${name}\" not found in ${MAP}")
  endif()
  math(EXPR size "0x${CMAKE_MATCH_1}" OUTPUT_FORMAT DECIMAL)
  set(${out} ${size} PARENT_SCOPE)
endfunction()

section_size("Golem driver" rom)
section_size("Golem state" wram)
message("driver ROM: ${rom} bytes (limit ${MAX_ROM}), WRAM: ${wram} bytes (limit ${MAX_WRAM})")

if(rom GREATER MAX_ROM OR wram GREATER MAX_WRAM)
  message(FATAL_ERROR "driver size over its budget (docs/driver-steps/07-budgets.md)")
endif()
