set(URBI_ROOT_DIR "/usr" CACHE PATH "Urbi root directory.")

find_package (Urbi REQUIRED)
#find_package (Boost REQUIRED thread)

link_directories (${BOOST_LIBRARYDIR} /home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Lib)

#include_directories (${URBI_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} /home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Include)
include_directories (${URBI_INCLUDE_DIRS} /home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Include)

add_library (UKinectCamera SHARED UKinectCamera.cpp OpenNiKinectSingelton.cpp UKinectModule.cpp)

#target_link_libraries (UKinectCamera  ${URBI_LIBRARIES}  ${Boost_LIBRARIES} OpenNI)
target_link_libraries (UKinectCamera  ${URBI_LIBRARIES}  OpenNI)

set_target_properties (UKinectCamera PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1)

install (TARGETS UKinectCamera  DESTINATION lib/gostai/uobjects COMPONENT libraries)
