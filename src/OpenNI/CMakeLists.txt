find_package (OpenNI REQUIRED)

include_directories (${OpenNI_INCLUDE_DIRS})

add_library (Kinect SHARED Kinect.cpp)

target_link_libraries (Kinect  ${OpenNI_LIBRARIES})

set_target_properties (Kinect PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1)