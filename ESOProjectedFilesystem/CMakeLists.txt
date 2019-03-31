add_executable(ESOProjectedFilesystem
	Inode.cpp
	Inode.h
	main.cpp
	ProjectedFilesystem.cpp
	ProjectedFilesystem.h
)
target_link_libraries(ESOProjectedFilesystem PRIVATE CLI11 ESOData ProjectedFSlib)
target_compile_definitions(ESOProjectedFilesystem PRIVATE -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -D_VC_EXTRALEAN -DNOMINMAX)
