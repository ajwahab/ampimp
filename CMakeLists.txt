cmake_minimum_required(VERSION 3.5)

# Uncomment ONLY one (1) project to build

set(AD5940_PROJECT "AD5940_ADC")
#set(AD5940_PROJECT "AD5940_Amperometric")
#set(AD5940_PROJECT "AD5940_BATImpedance")
#set(AD5940_PROJECT "AD5940_BIA")
#set(AD5940_PROJECT "AD5940_BioElec")
#set(AD5940_PROJECT "AD5940_BIOZ")
#set(AD5940_PROJECT "AD5940_ChronoAmperometric")
#set(AD5940_PROJECT "AD5940_DFT")
#set(AD5940_PROJECT "AD5940_ECG")
#set(AD5940_PROJECT "AD5940_ECSns_EIS")
#set(AD5940_PROJECT "AD5940_EDA")
#set(AD5940_PROJECT "AD5940_HSDACCal")
#set(AD5940_PROJECT "AD5940_Impedance")
#set(AD5940_PROJECT "AD5940_LPDAC")
#set(AD5940_PROJECT "AD5940_LPLoop")
#set(AD5940_PROJECT "AD5940_Ramp")
#set(AD5940_PROJECT "AD5940_Reset")
#set(AD5940_PROJECT "AD5940_Sequencer")
#set(AD5940_PROJECT "AD5940_SPI")
#set(AD5940_PROJECT "AD5940_SqrWaveVoltammetry")
#set(AD5940_PROJECT "AD5940_Temperature")
#set(AD5940_PROJECT "AD5940_WG")

project(${AD5940_PROJECT} C ASM)

# Linker script
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/lib/bsp/ADuCM302x_DFP.3.2.0/Source/GCC/ADuCM3029.ld")

set(CMSIS_PACK_DEVICE_DEFINES
        "-D__ADUCM3029__"
        "-D__PROGRAM_START" #to prevent CMSIS defaults from conflicting with those form ADI
        )

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -Wall -Wdouble-promotion")

# Include paths
set(CMSIS_PACK_DEVICE_INCLUDES
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/config"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/sys"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/rtos_map"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/adc"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/beep"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/crc"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/crypto"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/dma"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/flash"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/general"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/gpio"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/i2c"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/pwr"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/rng"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/rtc"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/spi"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/sport"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/tmr"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/uart"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/wdt"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Include/drivers/xint"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/adc"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/beep"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/crc"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/crypto"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/dma"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/flash"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/gpio"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/i2c"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/pwr"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/rng"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/rtc"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/spi"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/sport"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/tmr"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/uart"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/wdt"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/drivers/xint"
        )
set(CMSIS_PACK_INCLUDES "lib/CMSIS/CMSIS_5/CMSIS/Core/Include")

# Source files
set(CMSIS_PACK_DEVICE_SOURCES
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/system_ADuCM3029.c"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/retarget_uart.c"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/common.c"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/adi_cyclecount.c"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/GCC/startup_ADuCM3029.c"
        "lib/bsp/ADuCM302x_DFP.3.2.0/Source/GCC/reset_ADuCM3029.S"
        )

# Include application-specific project toolchain
if(AD5940_PROJECT STREQUAL "AD5940_Ramp")
  include(cmake/AD5940_Ramp.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_SqrWaveVoltammetry")
  include(cmake/AD5940_SqrWaveVoltammetry.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_ADC")
  include(cmake/AD5940_ADC.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_ChronoAmperometric")
  include(cmake/AD5940_ChronoAmperometric.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_BATImpedance")
  include(cmake/AD5940_BATImpedance.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_SPI")
  include(cmake/AD5940_SPI.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_LPDAC")
  include(cmake/AD5940_LPDAC.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_BIOZ")
  include(cmake/AD5940_BIOZ-2Wire.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_Reset")
  include(cmake/AD5940_Reset.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_HSDACCal")
  include(cmake/AD5940_HSDACCal.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_Temperature")
  include(cmake/AD5940_Temperature.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_Amperometric")
  include(cmake/AD5940_Amperometric.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_BioElec")
  include(cmake/AD5940_BioElec.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_EDA")
  include(cmake/AD5940_EDA.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_ECSns_EIS")
  include(cmake/AD5940_ECSensEIS.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_BIA")
  include(cmake/AD5940_BIA.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_DFT")
  include(cmake/AD5940_DFT.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_WG")
  include(cmake/AD5940_WG.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_LPLoop")
  include(cmake/AD5940_LPLoop.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_ECG")
  include(cmake/AD5940_ECG.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_Sequencer")
  include(cmake/AD5940_Sequencer.cmake)
elseif(AD5940_PROJECT STREQUAL "AD5940_Impedance")
  include(cmake/AD5940_Impedance.cmake)
endif()

set(AD5940_PROJECT_SOURCES
    "lib/ad5940-examples/examples/ad5940lib/ad5940.c"
    "lib/ad5940-examples/examples/AD5940_ADC/ADICUP3029/ADICUP3029Port.c"
    "lib/ad5940-examples/examples/AD5940_ADC/ADICUP3029/main.c"
    )

set(AD5940_PROJECT_INCLUDES
    "lib/ad5940-examples/examples/ad5940lib"
    "${CMSIS_PACK_DEVICE_INCLUDES}"
    "${CMSIS_PACK_INCLUDES}"
    )

set(AD5940_PROJECT_DEFINITIONS
    "-DCHIPSEL_594X"
    "-D${CMSIS_PACK_DEVICE_DEFINES}"
    "-D_RTE_"
    )

include(cmake/aducm3029.cmake)
include(cmake/gdb-helper.cmake)
include(cmake/binutils-arm-none-eabi.cmake)

# ARM Math library
# link_directories("lib/CMSIS/CMSIS_5/CMSIS/DSP/Lib/GCC")

# Configure target
add_executable(${AD5940_PROJECT} ${CMSIS_PACK_DEVICE_SOURCES} ${AD5940_PROJECT_SOURCES})
string(TOLOWER "${AD5940_PROJECT}" PROJECT_LOWERCASE)
set_target_properties(${AD5940_PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT_LOWERCASE}.elf")
target_include_directories(${AD5940_PROJECT} PUBLIC ${AD5940_PROJECT_INCLUDES})
target_compile_definitions(${AD5940_PROJECT} PUBLIC ${AD5940_PROJECT_DEFINITIONS})
# target_link_libraries(${AD5940_PROJECT} libarm_cortexM3l_math.a)
create_bin_output(${AD5940_PROJECT})
generate_run_gdb_jlink(${AD5940_PROJECT})
print_section_sizes(${AD5940_PROJECT})
