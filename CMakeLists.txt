cmake_minimum_required(VERSION 3.28)
project(aspen LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(D "${CMAKE_BINARY_DIR}/Dependencies" CACHE STRING
  "Path to dependencies folder.")
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
    STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
if(CMAKE_CONFIGURATION_TYPES AND
    NOT "MinSizeRel" IN_LIST CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};MinSizeRel"
    CACHE STRING "Available build configurations." FORCE)
endif()
if(WIN32)
  set(configure_script
    cmd /c CALL "${CMAKE_SOURCE_DIR}/configure.bat" -DD "${D}"
    "${CMAKE_BUILD_TYPE}")
elseif(UNIX)
  set(configure_script "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    "${CMAKE_BUILD_TYPE}")
endif()
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "ASPEN_SKIP_CMAKE=1" ${configure_script}
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" RESULT_VARIABLE configure_result
  OUTPUT_VARIABLE configure_output ERROR_VARIABLE configure_error
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
if(NOT configure_result EQUAL 0)
  message(FATAL_ERROR "Configuration script failed with error:\n"
    "${configure_error}\nOutput:\n${configure_output}")
endif()
include(Config/dependencies.cmake)
set(TEST_INSTALL_DIRECTORY "${PROJECT_BINARY_DIR}/Tests")
set(LIB_INSTALL_DIRECTORY "${PROJECT_BINARY_DIR}/Libraries")
set(ASPEN_SANITIZER "none" CACHE STRING "The sanitizer to build with.")
set_property(CACHE ASPEN_SANITIZER PROPERTY STRINGS "none" "address" "thread")
enable_testing()
if(MSVC)
  list(APPEND CMAKE_VS_GLOBALS
    UseMultiToolTask=true EnforceProcessCountAcrossBuilds=true
    "CL_MPCount=$([System.Environment]::ProcessorCount)")
  add_compile_options(/bigobj /external:anglebrackets /external:W0
    $<$<CONFIG:Release>:/GL> /MP /WX /Zc:__cplusplus /Zc:preprocessor)
  add_compile_definitions(_CRT_SECURE_NO_DEPRECATE NOMINMAX
    _SCL_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
  add_link_options($<$<CONFIG:Release>:/LTCG>)
  if(ASPEN_SANITIZER STREQUAL "thread")
    message(FATAL_ERROR "ThreadSanitizer is not available on Windows.")
  elseif(NOT ASPEN_SANITIZER STREQUAL "none")
    add_compile_options(/fsanitize=${ASPEN_SANITIZER})
  endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Werror -g $<$<CONFIG:Release>:-DNDEBUG>)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-fsized-deallocation)
  endif()
  if(NOT ASPEN_SANITIZER STREQUAL "none")
    add_compile_options(-fsanitize=${ASPEN_SANITIZER} -fno-omit-frame-pointer)
    add_link_options(-fsanitize=${ASPEN_SANITIZER})
  endif()
endif()
if(CYGWIN)
  add_compile_definitions(__USE_W32_SOCKETS)
endif()
file(GLOB aspen_header_files ${ASPEN_INCLUDE_PATH}/Aspen/*.hpp)
source_group("Header Files" FILES ${aspen_header_files})
include_directories(${ASPEN_INCLUDE_PATH})
add_custom_target(aspen SOURCES ${aspen_header_files})
add_subdirectory(Config/ConcurrencyTests)
add_subdirectory(Config/Python)
add_subdirectory(Config/Tests)
set(clean_outputs)
foreach(target python aspen_tester aspen_concurrency_tester)
  list(APPEND clean_outputs
    "$<TARGET_FILE:${target}>" "$<TARGET_OBJECTS:${target}>")
  if(MSVC)
    get_target_property(target_directory ${target} BINARY_DIR)
    set(compile_pdb_directory "${target_directory}/${target}.dir/$<CONFIG>")
    set_target_properties(${target} PROPERTIES COMPILE_PDB_NAME "${target}"
      COMPILE_PDB_OUTPUT_DIRECTORY "${compile_pdb_directory}")
    list(APPEND clean_outputs "$<TARGET_PDB_FILE:${target}>"
      "${compile_pdb_directory}/${target}.pdb"
      "$<TARGET_FILE_DIR:${target}>/$<TARGET_FILE_BASE_NAME:${target}>.ilk")
  endif()
endforeach()
list(APPEND clean_outputs "$<TARGET_LINKER_FILE:python>")
if(MSVC)
  list(APPEND clean_outputs
    "$<PATH:REPLACE_EXTENSION,LAST_ONLY,$<TARGET_LINKER_FILE:python>,.exp>")
endif()
configure_file(Config/clean.cmake CMakeFiles/clean.cmake.in @ONLY)
file(GENERATE
  OUTPUT "${PROJECT_BINARY_DIR}/CMakeFiles/aspen_clean_outputs_$<CONFIG>.cmake"
  INPUT "${PROJECT_BINARY_DIR}/CMakeFiles/clean.cmake.in")
