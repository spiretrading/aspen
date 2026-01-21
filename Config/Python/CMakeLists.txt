file(GLOB_RECURSE header_files ${ASPEN_INCLUDE_PATH}/Aspen/Python/*.hpp)
file(GLOB_RECURSE source_files ${ASPEN_SOURCE_PATH}/Python/*.cpp)
add_library(python SHARED ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(python PROPERTIES OUTPUT_NAME aspen)
target_compile_definitions(python PRIVATE ASPEN_BUILD_DLL)
target_include_directories(python SYSTEM PRIVATE
  ${PYBIND11_INCLUDE_PATH} ${PYTHON_INCLUDE_PATH})
if(MSVC)
  set_target_properties(python PROPERTIES SUFFIX ".pyd"
    LINK_FLAGS_RELEASE /LTCG)
endif()
if(UNIX)
  set_target_properties(python PROPERTIES PREFIX "")
endif()
set_target_properties(python PROPERTIES
  CXX_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN ON)
target_link_libraries(python PRIVATE
  $<$<CONFIG:Debug>:${PYTHON_LIBRARY_DEBUG_PATH}>
  $<$<NOT:$<CONFIG:Debug>>:${PYTHON_LIBRARY_OPTIMIZED_PATH}>)
install(TARGETS python CONFIGURATIONS Debug
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS python CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Release)
