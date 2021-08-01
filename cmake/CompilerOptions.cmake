function(create_compiler_opts target)
	set(WARN_LEVEL 3)

	# parse named arguments
	set(options "")
	set(args WARN)
	set(list_args DEFINE)
	cmake_parse_arguments(PARSE_ARGV 1 arg "${options}" "${args}" "${list_args}")

	# todo validate
	if (NOT ${arg_WARN} STREQUAL "")
		set(WARN_LEVEL ${arg_WARN})	
	endif()

	# GCC flags
	set(GCC_LINK_FLAGS
		$<$<CONFIG:Debug>:
			-Wl,--no-undefined>
		$<$<CONFIG:Release>:
			-Wl,--no-undefined
			-flto              # link time optimizations
			-O3                # max optimization
			-s>)               # strip symbols

	set(GCC_CXX_FLAGS
		-pipe
		-fPIC
		-fvisibility=hidden
		-fdiagnostics-color=always
		$<IF:$<STREQUAL:${WARN_LEVEL},0>,-w,-Wall>
		-Wno-comment
		$<$<CONFIG:Release>:
			-flto              # link time optimizations
			-O3                # max optimization 
			-ffast-math>       # fast floating point math
		$<$<CONFIG:Debug>:
			-Og                # supress optimizations
			-g                 # generate debug info
			-ggdb>)            # generate ggdb friendly debug info

	# MSVC flags
	set(MSVC_LINK_FLAGS 
		$<$<CONFIG:Release>:
			/LTCG>) # perform link time optimizations

	set(MSVC_CXX_FLAGS
		/wd4068                # ignore GCC pragmas
		/wd4267                # ignore integer narrowing warnings
		/wd4250                # ignore function hiding with virtual
		/EHsc                  # standard C++ exception handling
		/MP                    # build with Multiple Processes
		$<IF:$<STREQUAL:${WARN_LEVEL},0>,/W0,/W${WARN_LEVEL}>
		$<$<CONFIG:Release>:
			/MT                # use static runtime
			/O2                # max optimizations
			/GL                # full exe/dll optimization
			/Gy                # generate useful information for optimizer
			/Ob2               # let compiler inline freely
			/fp:fast>          # fast floating point math
		$<$<CONFIG:Debug>:
			/Ob0               # no inlining
			/Od                # no optimizations
			/Zi                # generate complete debug information
			/MDd               # link against dynamic runtime 
			/RTC1>)            # run-time checking

	add_library(${target} INTERFACE)
	target_compile_options(${target} INTERFACE $<IF:$<CXX_COMPILER_ID:MSVC>,${MSVC_CXX_FLAGS},${GCC_CXX_FLAGS}>)
	target_link_options(${target} INTERFACE $<IF:$<CXX_COMPILER_ID:MSVC>,${MSVC_LINK_FLAGS},${GCC_LINK_FLAGS}>)
	target_compile_definitions(${target} INTERFACE 
		$<$<CONFIG:Release>:NDEBUG>
		$<$<CONFIG:Debug>:_DEBUG>
		$<$<CXX_COMPILER_ID:MSVC>:
			WIN32_LEAN_AND_MEAN 
			_CRT_SECURE_NO_DEPRECATE 
			_CRT_SECURE_NO_WARNINGS 
			_CRT_NONSTDC_NO_WARNING
			_SCL_SECURE_NO_WARNINGS>
		${arg_DEFINE})
	target_compile_features(${target} INTERFACE cxx_std_11)
endfunction()
