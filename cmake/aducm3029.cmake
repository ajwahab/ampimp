##
## Author:   Adam Wahab / Peter Hinson / Johannes Brude
## License:  See LICENSE.TXT file included in the project
##
##
## ADuCM3029 target specific CMake file
##

if(NOT DEFINED LINKER_SCRIPT)
    message(FATAL_ERROR "No linker script defined")
endif(NOT DEFINED LINKER_SCRIPT)
message("Linker script: ${LINKER_SCRIPT}")

#---------------------------------------------------------------------------------------
# Set target specific compiler/linker flags
#---------------------------------------------------------------------------------------

# Object build options
# -mcpu=cortex-m3       Sepcifies target ARM processor.
# -march=armv7+nofp
# -mfloat-abi=soft      Allows the generation of code using software floating-point.

set(OBJECT_GEN_FLAGS "-mcpu=cortex-m3 -mthumb")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OBJECT_GEN_FLAGS}" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OBJECT_GEN_FLAGS}" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${OBJECT_GEN_FLAGS}" CACHE INTERNAL "ASM Compiler options")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T\"${LINKER_SCRIPT}\"" CACHE INTERNAL "Linker options")