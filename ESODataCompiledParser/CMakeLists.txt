add_library(ESODataCompiledParser STATIC
	${CMAKE_CURRENT_BINARY_DIR}/include/ESOData/Database/CompiledParser.h
	${CMAKE_CURRENT_BINARY_DIR}/CompiledParser.cpp
)

target_link_libraries(ESODataCompiledParser PUBLIC ESOData)
target_include_directories(ESODataCompiledParser PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include)

file(GLOB_RECURSE depends LIST_DIRECTORIES false ${PROJECT_SOURCE_DIR}/Directives/Database/*.dir)

add_custom_command(
	OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/include/ESOData/Database/CompiledParser.h
		${CMAKE_CURRENT_BINARY_DIR}/CompiledParser.cpp
	COMMAND cmake -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/include/ESOData/Database
	COMMAND
		ESODefCompiler
		${PROJECT_SOURCE_DIR}/Directives/Database
		${CMAKE_CURRENT_BINARY_DIR}/CompiledParser.cpp
		${CMAKE_CURRENT_BINARY_DIR}/include/ESOData/Database/CompiledParser.h
	DEPENDS
		${depends}
		$<TARGET_FILE:ESODefCompiler>
	VERBATIM
)

if(MSVC)
	target_compile_options(ESODataCompiledParser PRIVATE /bigobj)
endif()
