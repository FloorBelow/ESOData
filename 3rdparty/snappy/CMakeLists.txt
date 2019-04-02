add_library(snappy STATIC EXCLUDE_FROM_ALL
	        include/snappy.h
			include/snappy-sinksource.h
			include/snappy-stubs-public.h
			snappy.cc
			snappy-internal.h
			snappy-sinksource.cc
			snappy-stubs-internal.cc
			snappy-stubs-internal.h)
			
target_include_directories(snappy PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

if(MSVC)
	target_compile_options(snappy PRIVATE /wd4267 /wd4244)
endif()
