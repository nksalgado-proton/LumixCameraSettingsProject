cmake_minimum_required(VERSION 3.20)
project(tether_query LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# SDK headers and lib location
set(SDK_DIR "${CMAKE_SOURCE_DIR}/../../data/g9ii/LumixRemoteControlLibraryBeta1.00/Library")

# --- Original tether_query (dynamic loading of Tether DLL) ---
add_executable(tether_query main.cpp)
target_link_libraries(tether_query PRIVATE ole32)

# --- SDK test (links against official SDK DLL) ---
add_executable(sdk_test sdk_test.cpp)
target_include_directories(sdk_test PRIVATE "${SDK_DIR}")
target_link_directories(sdk_test PRIVATE "${SDK_DIR}")
target_link_libraries(sdk_test PRIVATE ole32 Lmxptpif)

# Copy SDK DLL to output directory
add_custom_command(TARGET sdk_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_DIR}/Lmxptpif.dll"
    "$<TARGET_FILE_DIR:sdk_test>/Lmxptpif.dll"
)

# --- Tether DLL test (dynamic load, ARRAY_MAX=64) ---
add_executable(tether_test tether_test.cpp)
target_link_libraries(tether_test PRIVATE ole32)

# --- Tether with SDK-sized struct (ARRAY_MAX=512) ---
add_executable(tether512_test tether512_test.cpp)
target_link_libraries(tether512_test PRIVATE ole32)

# --- Tether connection paths test ---
add_executable(tether_connect_test tether_connect_test.cpp)
target_link_libraries(tether_connect_test PRIVATE ole32)

# --- Tether OpenSession focus test ---
add_executable(tether_open_test tether_open_test.cpp)
target_link_libraries(tether_open_test PRIVATE ole32)

# --- Tether clean minimal test ---
add_executable(tether_clean_test tether_clean_test.cpp)
target_link_libraries(tether_clean_test PRIVATE ole32)

# --- Tether Search_Device first test ---
add_executable(tether_search_first tether_search_first.cpp)
target_link_libraries(tether_search_first PRIVATE ole32)

# --- Tether USB mode test ---
add_executable(tether_usb_mode tether_usb_mode.cpp)
target_link_libraries(tether_usb_mode PRIVATE ole32)

# --- Tether with message pump ---
add_executable(tether_msgpump tether_msgpump.cpp)
target_link_libraries(tether_msgpump PRIVATE ole32)

# --- Direct WPD/MTP test ---
add_executable(wpd_test wpd_test.cpp)
target_link_libraries(wpd_test PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD vendor operation probe ---
add_executable(wpd_vendor wpd_vendor.cpp)
target_link_libraries(wpd_vendor PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD vendor query test ---
add_executable(wpd_query wpd_query.cpp)
target_link_libraries(wpd_query PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD GetProperty/ListProperty probe ---
add_executable(wpd_getprop wpd_getprop.cpp)
target_link_libraries(wpd_getprop PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD comprehensive property scanner ---
add_executable(wpd_scan wpd_scan.cpp)
target_link_libraries(wpd_scan PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD deep property scanner ---
add_executable(wpd_deepscan wpd_deepscan.cpp)
target_link_libraries(wpd_deepscan PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD Fn button probe ---
add_executable(wpd_fnprobe wpd_fnprobe.cpp)
target_link_libraries(wpd_fnprobe PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD property snapshot for diffing ---
add_executable(wpd_snapshot wpd_snapshot.cpp)
target_link_libraries(wpd_snapshot PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD config mechanism scanner ---
add_executable(wpd_configscan wpd_configscan.cpp)
target_link_libraries(wpd_configscan PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD CamSet DAT file download/save ---
add_executable(wpd_camset wpd_camset.cpp)
target_link_libraries(wpd_camset PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD CamSet DAT fast download via directory traversal ---
add_executable(wpd_camset2 wpd_camset2.cpp)
target_link_libraries(wpd_camset2 PRIVATE ole32 PortableDeviceGUIDs)

# --- WPD object info dump ---
add_executable(wpd_objinfo wpd_objinfo.cpp)
target_link_libraries(wpd_objinfo PRIVATE ole32 PortableDeviceGUIDs)

# --- SetupCtrl / CamSet save probe ---
add_executable(wpd_setupctrl wpd_setupctrl.cpp)
target_link_libraries(wpd_setupctrl PRIVATE ole32 PortableDeviceGUIDs)

# --- Hybrid test (SDK session + Tether queries) ---
add_executable(hybrid_test hybrid_test.cpp)
target_include_directories(hybrid_test PRIVATE "${SDK_DIR}")
target_link_directories(hybrid_test PRIVATE "${SDK_DIR}")
target_link_libraries(hybrid_test PRIVATE ole32 Lmxptpif)
add_custom_command(TARGET hybrid_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDK_DIR}/Lmxptpif.dll"
    "$<TARGET_FILE_DIR:hybrid_test>/Lmxptpif.dll"
)
