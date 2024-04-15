cmake_minimum_required(VERSION 3.5...3.16)

project(ufopcl
	VERSION 2.0.0
	DESCRIPTION "The UFO pcl library"
	LANGUAGES CXX
)

Include(FetchContent)

FetchContent_Declare(
  ufomath
  GIT_REPOSITORY https://github.com/UnknownFreeOccupied/ufomath
  GIT_TAG        main
  GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(ufomath)

add_library(ufopcl INTERFACE)
add_library(UFO::PCL ALIAS ufopcl)

set_target_properties(ufopcl PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION ${PROJECT_VERSION_MAJOR}
	CXX_STANDARD 17
	CXX_EXTENSIONS OFF
)

target_link_libraries(ufopcl INTERFACE UFO::Math)

target_include_directories(ufopcl INTERFACE
	$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_compile_features(ufopcl INTERFACE cxx_std_17)

include(GNUInstallDirs)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Only enable tests when we're the root project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_subdirectory(tests)
endif()

install(TARGETS ufopcl
	EXPORT ufopclTargets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT ufopclTargets
	FILE "ufopclTargets.cmake"
	NAMESPACE UFO::
	DESTINATION lib/cmake/${PROJECT_NAME}
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${PROJECT_SOURCE_DIR}/cmake/ufopclConfig.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/ufopclConfig.cmake"
	INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/ufopclConfigVersion.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

INSTALL(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/ufopclConfig.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/ufopclConfigVersion.cmake"
	DESTINATION lib/cmake/${PROJECT_NAME}
)

install(
	DIRECTORY ${PROJECT_SOURCE_DIR}/include/
	DESTINATION include
)