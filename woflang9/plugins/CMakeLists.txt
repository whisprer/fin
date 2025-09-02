cmake_minimum_required(VERSION 3.20)

# collect only this folder's *_ops.cpp
file(GLOB PLUGIN_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*_ops.cpp")
message(STATUS "GLOB found these files: ${PLUGIN_SOURCES}")

if (PLUGIN_SOURCES STREQUAL "")
  message(WARNING "No *_ops.cpp plugin sources found in plugins/")
endif()

# disable unstable plugins for now
set(disabled_plugins
  chess_ops
  simple_chess_ops
  neural_chess_brain
  chemistry_ops
)

set(built_plugins)

foreach(src ${PLUGIN_SOURCES})
  get_filename_component(fname "${src}" NAME)
  get_filename_component(stem  "${src}" NAME_WE)

  list(FIND disabled_plugins "${stem}" _idx)
  if (_idx GREATER -1)
    message(STATUS "Skipping plugin (disabled): ${fname}")
    continue()
  endif()

  add_library(${stem} SHARED "${src}")
  target_link_libraries(${stem} PRIVATE woflang_core)

  target_include_directories(${stem} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/core
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  # keep DLLs in bin/plugins; import libs in build/lib
  set_target_properties(${stem} PROPERTIES
    PREFIX "lib"
    OUTPUT_NAME "${stem}"
    RUNTIME_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/bin/plugins"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/plugins"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/plugins"
    LIBRARY_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/bin/plugins"
    LIBRARY_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/bin/plugins"
    LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/plugins"
    ARCHIVE_OUTPUT_DIRECTORY         "${CMAKE_BINARY_DIR}/lib"
  )

  list(APPEND built_plugins ${stem})
endforeach()

if (built_plugins)
  message(STATUS "Plugins configured: ${built_plugins}")
endif()
