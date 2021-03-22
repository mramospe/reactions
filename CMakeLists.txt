cmake_minimum_required(VERSION 2.8)
project("Reactions" VERSION 0.0.0
DESCRIPTION "A header only c++ library to parse reaction strings"
LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

option(INSTALL_TESTS "Whether to install the test scripts or not" OFF)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
$<INSTALL_INTERFACE:include>)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}ConfigVersion.cmake"
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_package_config_file(
 "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
 "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Config.cmake"
 INSTALL_DESTINATION
 ${PROJECT_BINARY_DIR}/cmake)

export(EXPORT ${PROJECT_NAME}Targets
  FILE "${PROJECT_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake"
)

file(GLOB_RECURSE HEADER_FILES_TO_CONFIG ${PROJECT_SOURCE_DIR}/include/reactions/*.hpp.in)

install(FILES ${HEADER_FILES} DESTINATION include/reactions)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/python/reactions/data DESTINATION ${PROJECT_BINARY_DIR})
set(PDG_TABLE ${PROJECT_BINARY_DIR}/data/ParticleTable.txt)
foreach(include_file ${HEADER_FILES_TO_CONFIG})
  string(REPLACE ".in" "" file_name ${include_file})
  configure_file(${include_file} ${file_name})
endforeach()

file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/reactions/*.hpp)

install(FILES ${HEADER_FILES} DESTINATION ${PROJECT_BINARY_DIR}/include/reactions)

#
# Installation of the test scripts
#
if(INSTALL_TESTS)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/test)
    include_directories(include test/cpp)
    file(GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/test/cpp/*.cpp)
    set(CMAKE_CXX_FLAGS "-O3 -pedantic -Wall -Wextra")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -pedantic -Wall -Wextra")
    foreach(testsourcefile ${TEST_SOURCES})
      get_filename_component(testname ${testsourcefile} NAME_WE)
      add_executable(${testname} ${testsourcefile})
      set_target_properties(${testname} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
    endforeach(testsourcefile ${TEST_SOURCES})
endif(INSTALL_TESTS)
