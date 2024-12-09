option(UFO_CLOUD_DOCS     "Generate documentation" OFF)
option(UFO_CLOUD_TESTING  "Unit testing"           OFF)
option(UFO_CLOUD_COVERAGE "Test Coverage"          OFF)

add_library(Cloud INTERFACE)
add_library(UFO::Cloud ALIAS Cloud)

target_link_libraries(Cloud 
	INTERFACE 
		UFO::Utility
)

include(GNUInstallDirs)

target_include_directories(Cloud 
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

if(UFO_TESTING OR UFO_CLOUD_TESTING)
  add_subdirectory(tests)
endif()

if(UFO_DOCS OR UFO_CLOUD_DOCS)
	add_subdirectory(docs)
endif()

install(TARGETS Cloud EXPORT Cloud-targets
	COMPONENT Cloud
	LIBRARY  DESTINATION lib
	ARCHIVE  DESTINATION lib
	RUNTIME  DESTINATION bin
  INCLUDES DESTINATION include
)

install(EXPORT Cloud-targets
  FILE "Cloud-targets.cmake"
  NAMESPACE UFO::
  DESTINATION lib/cmake/${PROJECT_NAME}
	COMPONENT Cloud
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/Cloud-config-version.cmake"
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Cloud-config.cmake.in"
	"${CMAKE_CURRENT_BINARY_DIR}/Cloud-config.cmake"
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/Cloud-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Cloud-config-version.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  COMPONENT Cloud
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
	COMPONENT Cloud
	DESTINATION ${CMAKE_INSTALL_PREFIX}
)