#
# Copyright (c) 2018 - 2019 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

################################################################################

# Add custom board directory
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)

# Add PMW3389 driver module
list(APPEND ZEPHYR_EXTRA_MODULES
  ${CMAKE_CURRENT_LIST_DIR}/../modules/drivers/pmw3389
)

# Set application configuration directory
# set(APPLICATION_CONFIG_DIR "${CMAKE_CURRENT_LIST_DIR}/configuration/\${NORMALIZED_BOARD_TARGET}")

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project("nRF Desktop"
        VERSION 0.1)

################################################################################

assert(CONFIG_DESKTOP_HID_REPORT_DESC "HID report descriptor file must be specified")

# NORDIC SDK APP START
target_sources(app PRIVATE
  src/main.c
  ${CONFIG_DESKTOP_HID_REPORT_DESC}
  )
# NORDIC SDK APP END

# Include application events and configuration headers
zephyr_library_include_directories(
  src/events
  src/util
  )

zephyr_include_directories(
  configuration/common
  configuration/${NORMALIZED_BOARD_TARGET}
  )

# Application sources
add_subdirectory(src/events)
add_subdirectory(src/hw_interface)
add_subdirectory(src/modules)
add_subdirectory(src/util)

if(CONFIG_IMG_MANAGER)
  zephyr_library_link_libraries(MCUBOOT_BOOTUTIL)
endif()

if(CONFIG_DESKTOP_CONFIG_CHANNEL_ENABLE)
  zephyr_linker_sources(SECTIONS nrf_desktop.ld)
endif()
