# Writes a C++ source defining the bytes of a file as an array:
#   cmake -DINPUT=<file> -DOUTPUT=<file.cpp> -DSYMBOL=<name> -P EmbedFile.cmake
# defines `const unsigned char <name>[]` and `const std::size_t <name>Size` in namespace golem.

file(READ "${INPUT}" content HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," bytes "${content}")
# 16 bytes per line, so the generated source stays readable.
string(REGEX REPLACE "((0x..,){16})" "\\1\n    " bytes "${bytes}")
get_filename_component(name "${INPUT}" NAME)
file(WRITE "${OUTPUT}" "// Generated from ${name} by cmake/EmbedFile.cmake. Do not edit.\n"
  "#include <cstddef>\n\nnamespace golem {\n\n"
  "extern const unsigned char ${SYMBOL}[] = {\n    ${bytes}\n};\n"
  "extern const std::size_t ${SYMBOL}Size = sizeof ${SYMBOL};\n\n"
  "} // namespace golem\n")
