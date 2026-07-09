cmake_minimum_required(VERSION 3.16)
project(nm-helloworldvpn-plugin LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)
pkg_check_modules(LIBM REQUIRED IMPORTED_TARGET libnm)
pkg_check_modules(LIBMA REQUIRED IMPORTED_TARGET libnma-gtk4)
pkg_check_modules(GLIB REQUIRED glib-2.0)
# pkg_check_modules(NM REQUIRED networkmanager)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)

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

# NM_VERSION автоматически заполняется командой pkg_check_modules (например, "1.56.1")
if(NM_VERSION)
    # Разбиваем строку "1.56.1" на список [1, 56, 1]
    string(REPLACE "." ";" _NM_VER_LIST ${NM_VERSION})
    
    list(GET _NM_VER_LIST 0 NM_MAJOR_VERSION)
    list(GET _NM_VER_LIST 1 NM_MINOR_VERSION)
    
    # На случай, если версия вдруг будет "1.56" без микро-версии
    list(LENGTH _NM_VER_LIST _NM_VER_LEN)
    if(_NM_VER_LEN GREATER 2)
        list(GET _NM_VER_LIST 2 NM_MICRO_VERSION)
    else()
        set(NM_MICRO_VERSION 0)
    endif()
else()
    # Фоллбэк на случай, если версия вообще не определилась
    message(WARNING "Could not determine libnm version, using defaults")
    set(NM_MAJOR_VERSION 1)
    set(NM_MINOR_VERSION 0)
    set(NM_MICRO_VERSION 0)
endif()

# 3. Генерируем nm-version.h из вашего .in файла
# Укажите точный путь к файлу nm-version-macros.h.in, если он лежит не в корне
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/shared/nm-version-macros.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/nm-version.h
    #shared/nm-version-macros.h.in 
    # shared/nm-version.h
    @ONLY
)

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
	./
	 ${GLIB_INCLUDE_DIRS}
	 ${LIBM_INCLUDE_DIRS} 
	 ${CMAKE_CURRENT_BINARY_DIR}
	 shared/
	 shared/nm-utils
)
target_link_libraries(nm-utils PRIVATE
    ${GLIB_LIBRARIES}
    ${LIBM_LIBRARIES}      # <-- ДОБАВЬТЕ ЭТУ СТРОКУ
)

# Библиотека с виджетом (переиспользуемая)
# add_library(filewidget STATIC file-chooser-widget.c)
# target_include_directories(filewidget PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# target_link_libraries(filewidget PUBLIC PkgConfig::GTK4)

pkg_check_modules(LIBM REQUIRED IMPORTED_TARGET libnm gmodule-2.0)

# UI-плагин (динамическая библиотека для libnm)
set( SRC 
	  my-vpn-editor.c
	  my-vpn-editor-plugin.c
    	  ${CMAKE_CURRENT_BINARY_DIR}/resources.c   # <-- ДОБАВЬТЕ ЭТУ СТРОКУ
    )

add_library(target1 MODULE
	${SRC}
)

add_library(target2 MODULE
	${SRC}
)


target_link_libraries(target1 PRIVATE 
	#   filewidget 
	nm-utils
    PkgConfig::GTK4 
    PkgConfig::LIBM
    PkgConfig::LIBMA
)


target_link_libraries(target2 PRIVATE 
	#   filewidget 
    nm-utils
    PkgConfig::GTK4 
    PkgConfig::LIBM
    #  PkgConfig::LIBMA
)


set_target_properties(target1 PROPERTIES
	# PREFIX ""  # Без "lib" в имени
    OUTPUT_NAME "nm-gtk4-vpn-plugin-helloworld-editor"
    # OUTPUT_NAME ${PROJECT_NAME} 
)

target_include_directories(target1 PRIVATE 
	./
	shared/
	shared/nm-utils
)

set_target_properties(target2 PROPERTIES
	# PREFIX ""  # Без "lib" в имени
    OUTPUT_NAME "nm-vpn-plugin-helloworld"
    # OUTPUT_NAME ${PROJECT_NAME} 
)

target_include_directories(target2 PRIVATE 
	./
	shared/
	shared/nm-utils
)

# Установка
# install(TARGETS  ${PROJECT_NAME}
#        LIBRARY DESTINATION /usr/lib/NetworkManager/)
# install(FILES my-vpn-service.name 
#        DESTINATION /usr/lib/NetworkManager/VPN/)
