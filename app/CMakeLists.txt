cmake_minimum_required(VERSION 3.20.0)
set(BOARD rpi_pico)
set(DTC_OVERLAY_FILE protocon.overlay)

find_package(Zephyr)
project(protocon)

include_directories(src)
target_sources(app PRIVATE src/main.c)

set_property(GLOBAL APPEND PROPERTY HEX_FILES_TO_MERGE
	${app_bootloader_hex}
	${PROJECT_BINARY_DIR}/${KERNEL_HEX_NAME}
	${app_provision_hex})
