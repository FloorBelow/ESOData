cmake_minimum_required(VERSION 3.13)
project(ESOData)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

STRING(REPLACE "/INCREMENTAL" "" replacementFlags ${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO})
SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/incremental:no ${replacementFlags}" CACHE STRING "Flags used by the linker during RELWITHDEBINFO builds." FORCE)
STRING(REPLACE "/INCREMENTAL" "" replacementFlags ${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO})
SET(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/incremental:no ${replacementFlags}" CACHE STRING "Flags used by the linker during the creation of shared libraries during RELWITHDEBINFO builds." FORCE)

find_program(RUBY_EXECUTABLE ruby)
if(NOT RUBY_EXECUTABLE)
	message(FATAL_ERROR "Ruby is required")
endif()

add_subdirectory(archiveparse)

add_subdirectory(3rdparty)
add_subdirectory(ESOData)
if(${CMAKE_PROJECT_NAME} STREQUAL ${PROJECT_NAME})
	add_subdirectory(ESOData-test)
	add_subdirectory(ESOProjectedFilesystem)
endif()
add_subdirectory(ESODefCompiler)
add_subdirectory(ESODataCompiledParser)
