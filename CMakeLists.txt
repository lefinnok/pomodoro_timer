cmake_minimum_required(VERSION 3.0.0)
project(timer VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()


find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(timer main.cpp)
target_link_libraries(timer ${CURSES_LIBRARIES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
include(CPack)