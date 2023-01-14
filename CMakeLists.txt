cmake_minimum_required(VERSION 3.3)

project(RVEmu VERSION 1.0)

set(CURSES_NEED_NCURSES TRUE)

find_package(Curses REQUIRED)

if (!${CURSES_HAVE_NCURSES_H} AND !${CURSES_HAVE_NCURSES_NCURSES_H})
    message(FATAL_ERROR "Could not find ncurses headers.")
endif ()

include_directories(${CURSES_INCLUDE_DIRS})

add_executable(rv32-emulator "")
add_subdirectory(src)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

target_link_libraries(rv32-emulator ${CURSES_LIBRARIES})
add_compile_options(${CURSES_CFLAGS})