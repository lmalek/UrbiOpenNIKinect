set(URBI_ROOT_DIR "/usr" CACHE PATH "Urbi root directory.")

find_package (Urbi REQUIRED)
find_package (OpenCV REQUIRED)
find_package (OpenNI REQUIRED)

# You can place your OpenNI lib path here
#link_directories (/home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Lib)

# You can place your OpenNI include path here
#include_directories (${URBI_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} /home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Include)

include_directories (${URBI_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS} ${OpenNI_INCLUDE_DIRS})

add_library (UKinectOpenNI MODULE UKinectOpenNI.cpp)

target_link_libraries (UKinectOpenNI  ${URBI_LIBRARIES} ${OpenCV_CV_LIBRARY} ${OpenNI_LIBRARIES})

set_target_properties (UKinectOpenNI PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1
  PREFIX "")

#install (TARGETS UKinectOpenNI  DESTINATION lib/gostai/uobjects COMPONENT libraries)
