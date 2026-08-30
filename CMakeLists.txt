cmake_minimum_required(VERSION 3.20)
project(ASRTU_SSDV_Viewer VERSION 1.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets Network)

# Vendored SSDV codec (GPL-3.0-or-later; see third_party/ssdv_dslwp/COPYING).
# Linking this in means the resulting ASRTU_SSDV_Viewer binary as a whole is
# subject to GPL-3.0-or-later terms, regardless of the license on the rest
# of this repository's own code -- see README.md for details.
add_library(ssdv_dslwp STATIC
    third_party/ssdv_dslwp/ssdv.c
    third_party/ssdv_dslwp/rs8.c)
target_include_directories(ssdv_dslwp PUBLIC third_party/ssdv_dslwp)

configure_file(apps/ssdv_viewer/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/version.h @ONLY)

add_executable(ASRTU_SSDV_Viewer
    apps/ssdv_viewer/main.cpp
    apps/ssdv_viewer/ssdv_viewer_window.cpp
    apps/ssdv_viewer/ssdv_viewer_window.h
    apps/dsp/ssdv_image_window.cpp
    apps/dsp/ssdv_image_window.h
    libs/demod/ssdv_receiver.cpp
    libs/demod/ssdv_receiver.h
    libs/proxy/pmt_frame_decoder.h
    libs/common/translation.cpp
    libs/common/translation.h)
target_include_directories(ASRTU_SSDV_Viewer PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
    apps/dsp
    apps/ssdv_viewer
    libs/demod
    libs/proxy
    libs/common)
target_link_libraries(ASRTU_SSDV_Viewer PRIVATE
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network ssdv_dslwp)

install(TARGETS ASRTU_SSDV_Viewer RUNTIME DESTINATION bin)
install(FILES assets/translations/asrtu_en.qm assets/translations/asrtu_ja.qm
    DESTINATION bin/translations)
