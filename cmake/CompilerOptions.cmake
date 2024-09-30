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
		-Wl,--no-undefined
		$<$<CONFIG:Release>:
			-flto=auto			# link time optimizations
			-O3					# max optimization
			-s>)				# strip symbols

	set(GCC_CXX_FLAGS
		-pipe
		-fPIC
		-fvisibility=hidden
		-fdiagnostics-color=always
		$<IF:$<STREQUAL:${WARN_LEVEL},0>,-w,-Wall -Wextra -Wpedantic -Wcast-qual>
		-Wno-unused-parameter
		-Wno-missing-field-initializers
		$<$<CONFIG:Release>:
			-flto=auto			# link time optimizations
			-O3					# max optimization
			-ffast-math>		# fast floating point math
		$<$<CONFIG:Debug>:
			-O0					# suppress optimizations
			-g3					# generate debug info
			-ggdb3>)			# generate gdb friendly debug info

	# Clang flags
	set(CLANG_LINK_FLAGS
		$<IF:$<PLATFORM_ID:Darwin>,-Wl$<COMMA>-undefined$<COMMA>error,-Wl$<COMMA>--no-undefined>
		$<$<CONFIG:Release>:
			-flto								# link time optimizations
			-O3									# max optimization
			$<IF:$<PLATFORM_ID:Darwin>,,-s>>)	# strip symbols

	set(CLANG_CXX_FLAGS
		-pipe
		-fPIC
		-fvisibility=hidden
		-fdiagnostics-color=always
		$<IF:$<STREQUAL:${WARN_LEVEL},0>,-w,-Wall -Wextra -Wpedantic -Wcast-qual>
		-Wno-unused-parameter
		-Wno-missing-field-initializers
		$<$<CONFIG:Release>:
			-O3					# max optimization
			-flto				# link time optimizations
			-ffast-math>		# fast floating point math
		$<$<CONFIG:Debug>:
			-O0					# suppress optimizations
			-g3					# generate debug info
			-ggdb3>)			# generate gdb friendly debug info

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

	if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		target_compile_options(${target} INTERFACE ${MSVC_CXX_FLAGS})
		target_link_options(${target} INTERFACE ${MSVC_LINK_LAGS})
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
		target_compile_options(${target} INTERFACE ${CLANG_CXX_FLAGS})
		target_link_options(${target} INTERFACE ${CLANG_LINK_FLAGS})
	else() # default to GCC
		target_compile_options(${target} INTERFACE ${GCC_CXX_FLAGS})
		target_link_options(${target} INTERFACE ${GCC_LINK_FLAGS})
	endif()
	
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

	target_compile_features(${target} INTERFACE cxx_std_14)
endfunction()
