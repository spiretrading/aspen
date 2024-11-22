if(WIN32)
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.windows.cmake")
else()
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.posix.cmake")
endif()
set(ASPEN_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Include")
set(ASPEN_MODULES_PATH "${CMAKE_CURRENT_LIST_DIR}/../Modules")
set(DOCTEST_INCLUDE_PATH "${PROJECT_BINARY_DIR}/Dependencies/doctest-2.4.9")
set(PYBIND11_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/pybind11-2.10.3/include")
