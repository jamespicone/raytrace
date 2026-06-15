cmake_minimum_required(VERSION 3.15)
project(raytrace CXX)

# Default to a Release build for single-config generators (Ninja, Makefiles).
# Multi-config generators (Visual Studio) pick the config at build time.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_executable(raytrace
	src/main.cpp
	src/vector.cpp
	src/ray.cpp
	src/triangle.cpp
	src/vector.h
	src/ray.h
	src/triangle.h
	src/colour.h
)

target_compile_features(raytrace PRIVATE cxx_std_17)

# std::thread needs a threading library on POSIX; harmless on Windows.
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(raytrace PRIVATE Threads::Threads)

# --- SDL2 --------------------------------------------------------------------
# Prefer a system/package SDL2 (Linux, vcpkg, etc.); otherwise fall back to the
# vendored MSVC development libraries checked into the repo.
find_package(SDL2 QUIET)
if(SDL2_FOUND)
	target_link_libraries(raytrace PRIVATE SDL2::SDL2 SDL2::SDL2main)
else()
	set(SDL2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/SDL2-2.0.10")
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(SDL2_ARCH x64)
	else()
		set(SDL2_ARCH x86)
	endif()
	set(SDL2_LIBDIR "${SDL2_ROOT}/lib/${SDL2_ARCH}")

	target_include_directories(raytrace PRIVATE "${SDL2_ROOT}/include")
	target_link_libraries(raytrace PRIVATE
		"${SDL2_LIBDIR}/SDL2.lib"
		"${SDL2_LIBDIR}/SDL2main.lib"
	)

	# Copy the runtime DLL next to the executable so it can be launched.
	add_custom_command(TARGET raytrace POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			"${SDL2_LIBDIR}/SDL2.dll" "$<TARGET_FILE_DIR:raytrace>"
		COMMENT "Copying SDL2.dll to output directory"
	)
endif()

# --- Optimisation flags (match the Visual Studio Release config) -------------
if(MSVC)
	target_compile_options(raytrace PRIVATE
		$<$<CONFIG:Release>:/fp:fast /arch:AVX2>)
	set_target_properties(raytrace PROPERTIES
		INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
else()
	target_compile_options(raytrace PRIVATE
		$<$<CONFIG:Release>:-O3 -ffast-math -march=native>)
endif()
