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

FIND_PATH(OpenNI_INCLUDE_DIRS XnOpenNI.h /usr/include/ /usr/include/ni /usr/local/include/ /usr/local/include/ni)

FIND_PATH(OpenNI_LIBRARY_DIRS libOpenNI.so  /usr/lib /usr/local/lib)

FIND_LIBRARY(OpenNI_LIBRARIES NAMES OpenNI PATH ${OpenNI_LIBRARY_DIRS})
set(OpenNI_FOUND TRUE)
