include_directories(SYSTEM ${PYBIND11_INCLUDE_PATH})
include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})
file(GLOB header_files ${ASPEN_INCLUDE_PATH}/Aspen/Python/*.hpp)
file(GLOB module_files "${ASPEN_MODULES_PATH}/Python/*.ixx"
  "${ASPEN_MODULES_PATH}/Python/*.cpp")
add_library(python SHARED ${header_files} ${module_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(python PROPERTIES OUTPUT_NAME aspen)
target_compile_definitions(python PRIVATE ASPEN_BUILD_DLL)
set_target_properties(python PROPERTIES CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN 1)
add_definitions(-DASPEN_BUILD_DLL)
if(MSVC)
  target_compile_options(python PRIVATE /Zc:preprocessor)
  set_target_properties(python PROPERTIES SUFFIX ".pyd"
    LINK_FLAGS_RELEASE /LTCG)
endif()
if(UNIX)
  set_target_properties(python PROPERTIES PREFIX "")
endif()
set_source_files_properties(${module_files} PROPERTIES LANGUAGE CXX
  HEADER_FILE_ONLY FALSE)
target_link_libraries(python
  debug ${PYTHON_LIBRARY_DEBUG_PATH}
  optimized ${PYTHON_LIBRARY_OPTIMIZED_PATH}
  aspen)
install(TARGETS python CONFIGURATIONS Debug
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS python CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Release)
