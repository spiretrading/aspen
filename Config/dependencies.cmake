if(WIN32)
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.windows.cmake")
else()
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.posix.cmake")
endif()
set(ASPEN_INCLUDE_PATH "${PROJECT_BINARY_DIR}/Include")
set(ASPEN_SOURCE_PATH "${PROJECT_BINARY_DIR}/Source")
set(CATCH2_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/Catch2-2.6.1/single_include")
set(PYBIND11_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/pybind11-2.2.4/include")
