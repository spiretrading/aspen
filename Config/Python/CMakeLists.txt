include_directories(SYSTEM ${PYBIND11_INCLUDE_PATH})
include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})
file(GLOB header_files ${ASPEN_INCLUDE_PATH}/Aspen/Python/*.hpp)
file(GLOB source_files ${ASPEN_SOURCE_PATH}/Python/*.cpp)
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
add_library(python SHARED ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(python PROPERTIES OUTPUT_NAME aspen)
add_definitions(-DASPEN_BUILD_DLL)
if(MSVC)
  set_target_properties(python PROPERTIES SUFFIX ".pyd" LINK_FLAGS /SAFESEH:NO
    LINK_FLAGS_RELEASE /LTCG)
endif()
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif()
if(UNIX)
  set_target_properties(python PROPERTIES PREFIX "")
endif()
target_link_libraries(python
  debug ${PYTHON_LIBRARY_DEBUG_PATH}
  optimized ${PYTHON_LIBRARY_OPTIMIZED_PATH})
install(TARGETS python CONFIGURATIONS Debug
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS python CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Release)
if(WIN32)
  add_custom_command(TARGET python PRE_BUILD COMMAND cmd /c
    "CALL ${CMAKE_SOURCE_DIR}\\configure.bat -DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
elseif(UNIX)
  add_custom_command(TARGET python PRE_BUILD COMMAND
    "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
endif()
