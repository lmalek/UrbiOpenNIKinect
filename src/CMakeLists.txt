set(URBI_ROOT_DIR "/usr" CACHE PATH "Urbi root directory.")

find_package (Urbi REQUIRED)

link_directories (${BOOST_LIBRARYDIR} /home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Lib)

include_directories (${URBI_INCLUDE_DIRS} /home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Include)

add_library (UKinectCamera SHARED UKinectCamera.cpp OpenNiKinectSingelton.cpp UKinectModule.cpp)
add_library (UKinectDepth SHARED UKinectDepth.cpp OpenNiKinectSingelton.cpp UKinectModule.cpp)
add_library (UKinectUser SHARED UKinectUser.cpp OpenNiKinectSingelton.cpp UKinectModule.cpp)
add_library (UKinectOpenNI SHARED UKinectUser.cpp OpenNiKinectSingelton.cpp UKinectModule.cpp)

target_link_libraries (UKinectCamera  ${URBI_LIBRARIES}  OpenNI)
target_link_libraries (UKinectDepth  ${URBI_LIBRARIES}  OpenNI)
target_link_libraries (UKinectUser  ${URBI_LIBRARIES}  OpenNI)
target_link_libraries (UKinectOpenNI  ${URBI_LIBRARIES}  OpenNI)

set_target_properties (UKinectCamera PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1)

set_target_properties (UKinectDepth PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1)

set_target_properties (UKinectUser PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1)

set_target_properties (UKinectOpenNI PROPERTIES
  VERSION 0.0.1
  SOVERSION 0.0.1)

install (TARGETS UKinectCamera  DESTINATION lib/gostai/uobjects COMPONENT libraries)
install (TARGETS UKinectDepth  DESTINATION lib/gostai/uobjects COMPONENT libraries)
install (TARGETS UKinectUser  DESTINATION lib/gostai/uobjects COMPONENT libraries)
install (TARGETS UKinectOpenNI  DESTINATION lib/gostai/uobjects COMPONENT libraries)
