# Function: DIRECTORY_COPY
# Copies the contents of a source directory to a destination directory during the CMake configuration phase.
# Only copies files if they differ from the destination, improving efficiency.
#
# Parameters:
#   SOURCE_DIR: The source directory to copy. Can be a relative or absolute path.
#   DESTINATION_DIR: The destination directory where files should be copied. Can also be relative or absolute.
#
# Example Usage:
#   DIRECTORY_COPY("${CMAKE_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/resources")
function(DIRECTORY_COPY SOURCE_DIR DESTINATION_DIR)
    # Ensure the provided paths are absolute
    get_filename_component(SOURCE_DIR_ABS "${SOURCE_DIR}" ABSOLUTE)
    get_filename_component(DESTINATION_DIR_ABS "${DESTINATION_DIR}" ABSOLUTE)

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E make_directory "${DESTINATION_DIR_ABS}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${SOURCE_DIR_ABS}" "${DESTINATION_DIR_ABS}"
    )
endfunction()
