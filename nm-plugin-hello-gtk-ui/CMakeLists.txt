cmake_minimum_required(VERSION 3.16)
project(nm-helloworldvpn-gtk-ui LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBM REQUIRED IMPORTED_TARGET libnm)
pkg_check_modules(GMODULE REQUIRED IMPORTED_TARGET gmodule-2.0)
pkg_check_modules(GTK3 REQUIRED IMPORTED_TARGET gtk+-3.0)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

set(NM_PLUGIN_DIR "/usr/lib/NetworkManager" CACHE PATH
    "NetworkManager plugin directory used by the runtime selector")

add_library(nm-vpn-plugin-helloworld MODULE
    nm-vpn-plugin-helloworld.c
    my-vpn-editor-plugin.c
    my-vpn-editor-plugin.h
)
target_compile_definitions(nm-vpn-plugin-helloworld PRIVATE
    PLUGINDIR="${NM_PLUGIN_DIR}"
)
target_link_libraries(nm-vpn-plugin-helloworld PRIVATE
    PkgConfig::LIBM
    PkgConfig::GMODULE
)
set_target_properties(nm-vpn-plugin-helloworld PROPERTIES
    PREFIX ""
    OUTPUT_NAME "libnm-vpn-plugin-helloworld"
)

set(EDITOR_SOURCES
    my-vpn-editor.c
    my-vpn-editor.h
)

add_library(nm-vpn-plugin-helloworld-editor MODULE ${EDITOR_SOURCES})
target_link_libraries(nm-vpn-plugin-helloworld-editor PRIVATE
    PkgConfig::LIBM
    PkgConfig::GTK3
)
set_target_properties(nm-vpn-plugin-helloworld-editor PROPERTIES
    PREFIX ""
    OUTPUT_NAME "libnm-vpn-plugin-helloworld-editor"
)

add_library(nm-gtk4-vpn-plugin-helloworld-editor MODULE ${EDITOR_SOURCES})
target_link_libraries(nm-gtk4-vpn-plugin-helloworld-editor PRIVATE
    PkgConfig::LIBM
    PkgConfig::GTK4
)
set_target_properties(nm-gtk4-vpn-plugin-helloworld-editor PROPERTIES
    PREFIX ""
    OUTPUT_NAME "libnm-gtk4-vpn-plugin-helloworld-editor"
)
