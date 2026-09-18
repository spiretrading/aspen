cmake_minimum_required(VERSION 3.28)
file(REAL_PATH "${CMAKE_CURRENT_LIST_DIR}/.." source_directory)
file(REAL_PATH "${DEPENDENCIES_DIRECTORY}" dependencies_directory)
set(cache_file
  "${dependencies_directory}/cache_files/aspen/configure_complete")

function(fingerprint result)
  file(GLOB inputs
    "${source_directory}/setup.*"
    "${source_directory}/Config/extract.cmake"
    "${dependencies_directory}/*/.aspen_*_complete"
    "${dependencies_directory}/cache_files/aspen/*.build_complete")
  list(APPEND inputs "${CMAKE_CURRENT_FUNCTION_LIST_FILE}")
  list(SORT inputs)
  set(contents)
  foreach(input IN LISTS inputs)
    file(SHA256 "${input}" hash)
    string(APPEND contents "${input}:${hash}\n")
  endforeach()
  string(SHA256 hash "${contents}")
  set(${result} "${hash}" PARENT_SCOPE)
endfunction()

fingerprint(current)
if(EXISTS "${cache_file}")
  file(READ "${cache_file}" previous)
  if(previous STREQUAL current)
    return()
  endif()
endif()
file(REMOVE "${cache_file}")
if(CMAKE_HOST_WIN32)
  set(command cmd /c CALL "${source_directory}/setup.bat")
else()
  set(command "${source_directory}/setup.sh")
endif()
execute_process(COMMAND ${command}
  WORKING_DIRECTORY "${dependencies_directory}" RESULT_VARIABLE status)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Dependency setup failed: ${status}")
endif()
fingerprint(current)
file(WRITE "${cache_file}" "${current}")
