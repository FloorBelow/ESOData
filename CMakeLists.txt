cmake_minimum_required(VERSION 3.13)
project(ESOData)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(archiveparse)

add_subdirectory(3rdparty)
add_subdirectory(ESOData)
add_subdirectory(ESOData-test)
add_subdirectory(ESOProjectedFilesystem)
