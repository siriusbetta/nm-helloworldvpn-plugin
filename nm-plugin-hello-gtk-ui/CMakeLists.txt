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
set( SRC 
	  my-vpn-editor.c
	  my-vpn-editor-plugin.c
    )

add_library(target1 MODULE
	${SRC}
)

add_library(target2 MODULE
	${SRC}
)

target_link_libraries(target1 PRIVATE 
    filewidget 
    PkgConfig::GTK4 
    PkgConfig::LIBM
    PkgConfig::LIBMA
)

target_link_libraries(target2 PRIVATE 
    filewidget 
    PkgConfig::GTK4 
    PkgConfig::LIBM
    #  PkgConfig::LIBMA
)
set_target_properties(target1 PROPERTIES
	# PREFIX ""  # Без "lib" в имени
    OUTPUT_NAME "nm-gtk4-vpn-plugin-helloworld-editor"
    # OUTPUT_NAME ${PROJECT_NAME} 
)
set_target_properties(target2 PROPERTIES
	# PREFIX ""  # Без "lib" в имени
    OUTPUT_NAME "nm-vpn-plugin-helloworld"
    # OUTPUT_NAME ${PROJECT_NAME} 
)

# Установка
# install(TARGETS  ${PROJECT_NAME}
#        LIBRARY DESTINATION /usr/lib/NetworkManager/)
# install(FILES my-vpn-service.name 
#        DESTINATION /usr/lib/NetworkManager/VPN/)
