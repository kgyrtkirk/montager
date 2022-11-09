cmake_minimum_required(VERSION 3.0.2)
project(montager)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin/")

execute_process(COMMAND gimptool-2.0 --cflags OUTPUT_VARIABLE GIMP2_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND gimptool-2.0 --libs OUTPUT_VARIABLE GIMP2_LIBS OUTPUT_STRIP_TRAILING_WHITESPACE)

set(CMAKE_CXX_FLAGS "-Wall -O0 -Wextra -g -ggdb3 -std=c++14 ${CMAKE_CXX_FLAGS} ${GIMP2_CFLAGS}")
add_definitions(-DBOOST_LOG_DYN_LINK)

# Configure CCache if available
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)


file(GLOB core_SRC
    "src/*.h"
    "src/interface.cpp"
    "src/render.cpp"
    "src/fd_layout.cpp"
    "src/types.cpp"
    "src/progress.cpp"
)


set(montager_SRC ${core_SRC} "src/main.cpp")
set(montager_test_SRC ${core_SRC} "src/test.cpp")
#file(GLOB montager_SRC
#)

FIND_PACKAGE( Boost 1.40 COMPONENTS program_options log log_setup iostreams chrono REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

add_executable(montager ${montager_SRC})
target_link_libraries(montager ${Boost_LIBRARIES} pthread util ${GIMP2_LIBS})
target_link_libraries(montager PkgConfig::deps)

add_executable(montager-test ${montager_test_SRC})
target_link_libraries(montager-test ${Boost_LIBRARIES} pthread util ${GIMP2_LIBS})
target_link_libraries(montager-test PkgConfig::deps)





