add_executable(ESODefCompiler
	ESODefCompiler.cpp
	ESODefCompiler.h
	main.cpp
)
target_link_libraries(ESODefCompiler PRIVATE ESOData)
set_target_properties(ESODefCompiler PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED TRUE)