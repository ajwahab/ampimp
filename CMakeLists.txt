cmake_minimum_required(VERSION 3.5)

# Uncomment ONLY one (1) project to build

#set(PROJECT "AD5940_ADC")
#set(PROJECT "AD5940_Amperometric")
#set(PROJECT "AD5940_BATImpedance")
#set(PROJECT "AD5940_BIA")
#set(PROJECT "AD5940_BioElec")
#set(PROJECT "AD5940_BIOZ")
#set(PROJECT "AD5940_ChronoAmperometric")
#set(PROJECT "AD5940_DFT")
#set(PROJECT "AD5940_ECG")
#set(PROJECT "AD5940_ECSns_EIS")
#set(PROJECT "AD5940_EDA")
#set(PROJECT "AD5940_HSDACCal")
#set(PROJECT "AD5940_Impedance")
#set(PROJECT "AD5940_LPDAC")
#set(PROJECT "AD5940_LPLoop")
#set(PROJECT "AD5940_Ramp")
#set(PROJECT "AD5940_Reset")
#set(PROJECT "AD5940_Sequencer")
#set(PROJECT "AD5940_SPI")
#set(PROJECT "AD5940_SqrWaveVoltammetry")
#set(PROJECT "AD5940_Temperature")
#set(PROJECT "AD5940_WG")
set(PROJECT "AMPIMP_APP")

project(${PROJECT} C ASM)

string(TOLOWER "${PROJECT}" PROJECT_LOWERCASE)
string(TOUPPER "${PROJECT}" PROJECT_UPPERCASE)

# Linker script
set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/GCC/ADuCM3029.ld")

# __PROGRAM_START: to prevent CMSIS defaults from conflicting with those form ADI
set(CMSIS_PACK_DEVICE_DEFINES
    ${CMSIS_PACK_DEVICE_DEFINES}
    "-D__ADUCM3029__"
    "-D__PROGRAM_START"
    )

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wdouble-promotion -O3 -g3")

# Include paths
set(CMSIS_PACK_DEVICE_INCLUDES
    ${CMSIS_PACK_DEVICE_INCLUDES}
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/config"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/sys"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/rtos_map"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/adc"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/beep"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/crc"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/crypto"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/dma"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/flash"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/general"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/gpio"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/i2c"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/pwr"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/rng"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/rtc"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/spi"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/sport"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/tmr"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/uart"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/wdt"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Include/drivers/xint"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/adc"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/beep"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/crc"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/crypto"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/dma"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/flash"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/gpio"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/i2c"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/pwr"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/rng"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/rtc"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/spi"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/sport"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/tmr"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/uart"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/wdt"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/drivers/xint"
    )

set(CMSIS_PACK_INCLUDES
    ${CMSIS_PACK_INCLUDES}
    "lib/CMSIS/CMSIS_5/CMSIS/Core/Include"
    )

# Source files
set(CMSIS_PACK_DEVICE_SOURCES
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/system_ADuCM3029.c"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/common.c"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/adi_cyclecount.c"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/GCC/startup_ADuCM3029.c"
    "lib/bsp/AnalogDevices.ADuCM302x_DFP.3.2.0/Source/GCC/reset_ADuCM3029.S"
    )

# Include application-specific project toolchain
set(AD5940_APP_SOURCES
    "lib/ad5940-examples/examples/AD5940_ADC/ADICUP3029/ADICUP3029Port.c"
    "lib/ad5940-examples/examples/AD5940_ADC/ADICUP3029/main.c"
    )
if(PROJECT STREQUAL "AD5940_Ramp")
  include(cmake/AD5940_Ramp.cmake)
elseif(PROJECT STREQUAL "AD5940_SqrWaveVoltammetry")
  include(cmake/AD5940_SqrWaveVoltammetry.cmake)
elseif(PROJECT STREQUAL "AD5940_ADC")
  include(cmake/AD5940_ADC.cmake)
elseif(PROJECT STREQUAL "AD5940_ChronoAmperometric")
  include(cmake/AD5940_ChronoAmperometric.cmake)
elseif(PROJECT STREQUAL "AD5940_BATImpedance")
  include(cmake/AD5940_BATImpedance.cmake)
elseif(PROJECT STREQUAL "AD5940_SPI")
  include(cmake/AD5940_SPI.cmake)
elseif(PROJECT STREQUAL "AD5940_LPDAC")
  include(cmake/AD5940_LPDAC.cmake)
elseif(PROJECT STREQUAL "AD5940_BIOZ")
  include(cmake/AD5940_BIOZ-2Wire.cmake)
elseif(PROJECT STREQUAL "AD5940_Reset")
  include(cmake/AD5940_Reset.cmake)
elseif(PROJECT STREQUAL "AD5940_HSDACCal")
  include(cmake/AD5940_HSDACCal.cmake)
elseif(PROJECT STREQUAL "AD5940_Temperature")
  include(cmake/AD5940_Temperature.cmake)
elseif(PROJECT STREQUAL "AD5940_Amperometric")
  include(cmake/AD5940_Amperometric.cmake)
elseif(PROJECT STREQUAL "AD5940_BioElec")
  set(AD5940_APP_SOURCES
      "lib/ad5940-examples/examples/AD5940_BioElec/ADICUP3029/ADICUP3029Port.c"
      "lib/ad5940-examples/examples/AD5940_BioElec/ADICUP3029/main.c"
      )
  include(cmake/AD5940_BioElec.cmake)
elseif(PROJECT STREQUAL "AD5940_EDA")
  set(AD5940_APP_SOURCES
      "lib/ad5940-examples/examples/AD5940_EDA/ADICUP3029/ADICUP3029Port.c"
      "lib/ad5940-examples/examples/AD5940_EDA/ADICUP3029/main.c"
      )
  include(cmake/AD5940_EDA.cmake)
elseif(PROJECT STREQUAL "AD5940_ECSns_EIS")
  include(cmake/AD5940_ECSensEIS.cmake)
elseif(PROJECT STREQUAL "AD5940_BIA")
  include(cmake/AD5940_BIA.cmake)
elseif(PROJECT STREQUAL "AD5940_DFT")
  include(cmake/AD5940_DFT.cmake)
elseif(PROJECT STREQUAL "AD5940_WG")
  include(cmake/AD5940_WG.cmake)
elseif(PROJECT STREQUAL "AD5940_LPLoop")
  include(cmake/AD5940_LPLoop.cmake)
elseif(PROJECT STREQUAL "AD5940_ECG")
  include(cmake/AD5940_ECG.cmake)
elseif(PROJECT STREQUAL "AD5940_Sequencer")
  include(cmake/AD5940_Sequencer.cmake)
elseif(PROJECT STREQUAL "AD5940_Impedance")
  include(cmake/AD5940_Impedance.cmake)
elseif(PROJECT STREQUAL "AMPIMP_APP")
  set(AMPIMP_APP_SOURCES
      "src/main.c"
      "src/retarget.c"
      "src/ampimp.c"
      "src/ADICUP3029Port.c"
      "src/uart_cmd.c"
      "lib/ad5940-examples/examples/AD5940_Amperometric/Amperometric.c"
      "lib/ad5940-examples/examples/AD5940_Impedance/Impedance.c"
      )
  set(AMPIMP_APP_INCLUDES
      "inc"
      "lib/ad5940-examples/examples/AD5940_Amperometric"
      "lib/ad5940-examples/examples/AD5940_Impedance"
      )
endif()

set(PROJECT_INCLUDES
    "lib/ad5940-examples/examples/ad5940lib"
    ${AMPIMP_APP_INCLUDES}
    ${CMSIS_PACK_DEVICE_INCLUDES}
    ${CMSIS_PACK_INCLUDES}
    )

set(PROJECT_SOURCES
    "lib/ad5940-examples/examples/ad5940lib/ad5940.c"
    ${AMPIMP_APP_SOURCES}
    ${CMSIS_PACK_DEVICE_SOURCES}
    )

set(PROJECT_DEFINITIONS
    "-DREDIRECT_OUTPUT_TO_UART"
    "-DCHIPSEL_594X"
    "-D_RTE_"
    ${CMSIS_PACK_DEVICE_DEFINES}
    )

include(cmake/aducm3029.cmake)
include(cmake/gdb-helper.cmake)
include(cmake/binutils-arm-none-eabi.cmake)

#add_link_options("-u _printf_float")
#add_link_options("-lc")

# ARM Math library
link_directories("lib/CMSIS/CMSIS_5/CMSIS/DSP/Lib/GCC")

# Configure target
add_executable(${PROJECT} ${PROJECT_SOURCES})
set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT_LOWERCASE}.elf")
target_include_directories(${PROJECT} PUBLIC ${PROJECT_INCLUDES})
target_compile_definitions(${PROJECT} PUBLIC ${PROJECT_DEFINITIONS})
# target_link_libraries(${PROJECT} libarm_cortexM3l_math.a)
create_bin_output(${PROJECT})
generate_run_gdb_jlink(${PROJECT})
print_section_sizes(${PROJECT})
