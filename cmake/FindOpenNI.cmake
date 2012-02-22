# FindOpenNI.cmake - Tries to find OpenNI
# This module sets those variables:
#
# OpenNI_FOUND                    
#
# OpenNI_INCLUDE_DIRS             
#
# OpenNI_LIBRARIES       
#
# OpenNI_LIBRARY_DIRS          

# ========================== #
# Find MOCKUP - FIX A.S.A.P. #
# ========================== #

FIND_PATH(OpenNI_INCLUDE_DIRS XnOpenNI.h /usr/include /usr/local/include ~/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Include)

FIND_PATH(OpenNI_LIBRARY_DIRS libOpenNI.so  /usr/include /usr/local/include ~/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Lib)

FIND_LIBRARY(OpenNI_LIBRARIES NAMES OpenNI PATH ${OpenNI_LIBRARY_DIRS})
set(OpenNI_FOUND TRUE)
