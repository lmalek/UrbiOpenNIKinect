/* 
 * File:   OpenNiKinectSingelton.cpp
 * Author: lmalek
 * 
 * Created on 7 marzec 2012, 15:38
 */

#include <XnStatus.h>
#include <sstream>
#include <stdexcept>

#include "OpenNiKinectSingelton.h"

xn::Context& OpenNIKinectSingelton::getContext() {
    return context;
}

OpenNIKinectSingelton::OpenNIKinectSingelton() {
    XnStatus nRetVal = context.Init();
    if (nRetVal != XN_STATUS_OK) { // failed to initializew
        std::stringstream ss;
        ss << "[OpenNI] Initialze context failed: " << xnGetStatusString(nRetVal);
        throw std::runtime_error(ss.str());
    }
}

OpenNIKinectSingelton::~OpenNIKinectSingelton() {
}

OpenNIKinectSingelton& OpenNIKinectSingelton::getInstance() {
    static OpenNIKinectSingelton instance;
    return instance;
}
