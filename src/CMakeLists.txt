set(URBI_ROOT_DIR "/usr" CACHE PATH "Urbi root directory.")

add_subdirectory (OpenNI)

find_package (Urbi REQUIRED)

include_directories (${URBI_INCLUDE_DIRS})

add_library (uopennikinect SHARED uopennikinect.cpp)

target_link_libraries (uopennikinect  ${URBI_LIBRARIES})

set_target_properties (uopennikinect PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1)

#install (TARGETS uopennikinect  DESTINATION lib/gostai/uobjects COMPONENT libraries)
