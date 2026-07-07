cmake_minimum_required(VERSION 3.16)
project(nm-helloworldvpn-plugin LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(LIBM REQUIRED IMPORTED_TARGET libnm)
pkg_check_modules(LIBMA REQUIRED IMPORTED_TARGET libnma-gtk4)

# Библиотека с виджетом (переиспользуемая)
add_library(filewidget STATIC file-chooser-widget.c)
target_include_directories(filewidget PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(filewidget PUBLIC PkgConfig::GTK4)

pkg_check_modules(LIBM REQUIRED IMPORTED_TARGET libnm gmodule-2.0)

# UI-плагин (динамическая библиотека для libnm)
add_library(${PROJECT_NAME} MODULE
    my-vpn-editor.c
    my-vpn-editor-plugin.c
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    filewidget 
    PkgConfig::GTK4 
    PkgConfig::LIBM
    PkgConfig::LIBMA
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    PREFIX ""  # Без "lib" в имени
    #    OUTPUT_NAME "nm-my-vpn-editor"
    OUTPUT_NAME ${PROJECT_NAME} 
)

# Установка
install(TARGETS  ${PROJECT_NAME}
        LIBRARY DESTINATION /usr/lib/NetworkManager/)
install(FILES my-vpn-service.name 
        DESTINATION /usr/lib/NetworkManager/VPN/)
