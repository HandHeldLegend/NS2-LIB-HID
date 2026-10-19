# NS2-LIB-HID — portable console-oriented helper for emulating a Nintendo Switch 2 controller over BLE.
#
# Licensing: see LICENSING.md for per-file terms.
#
# Copyright (c) 2026 Mitchell Cairns, Hand Held Legend, LLC.
#
# Build metadata in this file: CC BY-NC 4.0 (see LICENSING.md). SPDX-License-Identifier: CC-BY-NC-4.0
#
# Usage from a parent CMake project (after add_subdirectory):
#   target_link_libraries(<your_firmware> PRIVATE ns2_lib_hid)
#
# Public headers live in include/; compile this target as C11 or newer.

add_library(ns2_lib_hid STATIC
  ns2_lib_config.c
  ns2_lib_aes.c
  ns2_lib_memory.c
  ns2_lib_gatt.c
  ns2_lib_protocol.c
  ns2_lib_usb.c
  ns2_lib.c
)

target_include_directories(ns2_lib_hid PUBLIC
  "${CMAKE_CURRENT_LIST_DIR}/include"
)

target_compile_features(ns2_lib_hid PUBLIC c_std_11)

# Weak callback stubs in ns2_lib.c are meant to be overridden by the firmware / platform layer.
