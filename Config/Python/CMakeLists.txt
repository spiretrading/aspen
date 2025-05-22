include_directories(SYSTEM ${PYBIND11_INCLUDE_PATH})
include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})
file(GLOB_RECURSE header_files ${ASPEN_INCLUDE_PATH}/Aspen/Python/*.hpp)
file(GLOB_RECURSE source_files ${ASPEN_SOURCE_PATH}/Python/*.cpp)
add_library(python SHARED ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(python PROPERTIES OUTPUT_NAME aspen)
target_compile_definitions(python PRIVATE ASPEN_BUILD_DLL)
set_target_properties(python PROPERTIES CXX_VISIBILITY_PRESET default
  VISIBILITY_INLINES_HIDDEN 0)
if(MSVC)
  set_target_properties(python PROPERTIES SUFFIX ".pyd"
    LINK_FLAGS_RELEASE /LTCG)
endif()
target_compile_options(python PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fvisibility=hidden>)
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
