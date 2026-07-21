cmake_minimum_required(VERSION 3.16)
project(nm-helloworldvpn-plugin LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(LIBM REQUIRED IMPORTED_TARGET libnm)
pkg_check_modules(LIBNMA REQUIRED IMPORTED_TARGET libnma-gtk4)
pkg_check_modules(GLIB REQUIRED glib-2.0)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

# Генерация ресурсов
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources.c
    COMMAND ${GLIB_COMPILE_RESOURCES}
            --generate-source
            --target=${CMAKE_CURRENT_BINARY_DIR}/resources.c
            ${CMAKE_CURRENT_SOURCE_DIR}/my-vpn-editor.gresource.xml
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/my-vpn-editor.gresource.xml
            ${CMAKE_CURRENT_SOURCE_DIR}/my-vpn-editor.ui
    VERBATIM
)

# Версия libnm
if(NM_VERSION)
    string(REPLACE "." ";" _NM_VER_LIST ${NM_VERSION})
    list(GET _NM_VER_LIST 0 NM_MAJOR_VERSION)
    list(GET _NM_VER_LIST 1 NM_MINOR_VERSION)
    list(LENGTH _NM_VER_LIST _NM_VER_LEN)
    if(_NM_VER_LEN GREATER 2)
        list(GET _NM_VER_LIST 2 NM_MICRO_VERSION)
    else()
        set(NM_MICRO_VERSION 0)
    endif()
else()
    message(WARNING "Could not determine libnm version, using defaults")
    set(NM_MAJOR_VERSION 1)
    set(NM_MINOR_VERSION 0)
    set(NM_MICRO_VERSION 0)
endif()

# configure_file(
#     ${CMAKE_CURRENT_SOURCE_DIR}/shared/nm-version-macros.h.in
#     ${CMAKE_CURRENT_BINARY_DIR}/nm-version.h
#     @ONLY
# )

# NM utils (статическая библиотека)
set(NM_UTILS_SRC 
    shared/nm-utils/c-list-util.c
    shared/nm-utils/nm-dedup-multi.c
    shared/nm-utils/nm-enum-utils.c
    shared/nm-utils/nm-hash-utils.c
    shared/nm-utils/nm-random-utils.c
    shared/nm-utils/nm-shared-utils.c
    shared/nm-utils/nm-udev-utils.c
    shared/nm-utils/nm-vpn-plugin-utils.c
    shared/nm-utils/siphash24.c
)

add_library(nm-utils STATIC ${NM_UTILS_SRC})
target_compile_definitions(nm-utils PRIVATE _GNU_SOURCE)

target_include_directories(nm-utils PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${GLIB_INCLUDE_DIRS}
    ${LIBM_INCLUDE_DIRS}
    shared/
    shared/nm-utils
)

target_link_libraries(nm-utils PRIVATE
    ${GLIB_LIBRARIES}
    ${LIBM_LIBRARIES}
)

# === GTK4 UI Plugin (для gnome-control-center) ===
set(GTK4_SRC 
    my-vpn-editor.c
    my-vpn-editor-plugin.c
    ${CMAKE_CURRENT_BINARY_DIR}/resources.c
)

add_library(nm-gtk4-vpn-plugin-helloworld-editor MODULE ${GTK4_SRC})

target_include_directories(nm-gtk4-vpn-plugin-helloworld-editor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    shared/
    shared/nm-utils
)

target_link_libraries(nm-gtk4-vpn-plugin-helloworld-editor PRIVATE 
    nm-utils
    PkgConfig::GTK4 
    PkgConfig::LIBM
    PkgConfig::LIBNMA
)

set_target_properties(nm-gtk4-vpn-plugin-helloworld-editor PROPERTIES
    OUTPUT_NAME "nm-gtk4-vpn-plugin-helloworld-editor"
    PREFIX "lib"
)

# === .name файл ===
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/nm-helloworld-service.name.in
    ${CMAKE_CURRENT_BINARY_DIR}/nm-helloworld-service.name
    @ONLY
)

# Установка
install(TARGETS nm-gtk4-vpn-plugin-helloworld-editor
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/NetworkManager/)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nm-helloworld-service.name 
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/NetworkManager/VPN/)
