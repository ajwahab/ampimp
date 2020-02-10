##
## Author:  Johannes Bruder
## License: See LICENSE.TXT file included in the project
##
## CMake arm-none-eabi binutils integration and helper functions
##


#---------------------------------------------------------------------------------------
# Set tools
#---------------------------------------------------------------------------------------
set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objcopy${TOOLCHAIN_EXT})
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-objdump${TOOLCHAIN_EXT})
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-size${TOOLCHAIN_EXT})


#---------------------------------------------------------------------------------------
# Prints the section sizes
#---------------------------------------------------------------------------------------
function(print_section_sizes TARGET)
    get_target_property( BINARY_NAME ${TARGET} OUTPUT_NAME )
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${BINARY_NAME})
endfunction()

#---------------------------------------------------------------------------------------
# Creates output in hex format
#---------------------------------------------------------------------------------------
function(create_hex_output TARGET)
    get_target_property( BINARY_NAME ${TARGET} OUTPUT_NAME )
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Oihex ${BINARY_NAME} ${TARGET}.hex)
endfunction()

#---------------------------------------------------------------------------------------
# Creates output in binary format
#---------------------------------------------------------------------------------------
function(create_bin_output TARGET)
    get_target_property( BINARY_NAME ${TARGET} OUTPUT_NAME )
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -Obinary ${BINARY_NAME} ${TARGET}.bin)
endfunction()
