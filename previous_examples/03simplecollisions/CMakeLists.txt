cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

project (sdldemo CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${sdldemo_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)

find_package( OpenMP)
  if(OPENMP_FOUND)
     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
     set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
  endif()

include_directories(${SDL2_INCLUDE_DIR})
include_directories(SYSTEM ${OpenMP_INCLUDE_PATH})
include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/thirdparty/")

file(GLOB_RECURSE sdldemo_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE thirdparty_SOURCES "${PROJECT_SOURCE_DIR}/thirdparty/*.cpp")

add_executable(sdldemoapp ${sdldemo_SOURCES} ${thirdparty_SOURCES} )
target_link_libraries(sdldemoapp ${SDL2_LIBRARY})


add_custom_command(TARGET sdldemoapp PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:sdldemoapp>/data)