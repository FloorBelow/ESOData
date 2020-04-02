add_library(CLI11 INTERFACE)
target_include_directories(CLI11 INTERFACE CLI11)

add_subdirectory(snappy)
add_subdirectory(granny2)
add_subdirectory(DirectXTex)
