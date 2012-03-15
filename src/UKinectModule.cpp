/* 
 * File:   UKinectModule.cpp
 * Author: lmalek
 * 
 * Created on 7 marzec 2012, 15:39
 */

#include <iostream>

#include "UKinectModule.h"
#include "OpenNiKinectSingelton.h"

using namespace std;

UKinectModule::UKinectModule(const std::string& name) : UObject(name) {
    UBindFunction(UKinectModule, init); 
}

UKinectModule::~UKinectModule() {
}

UKinectModule::init() {
    cerr << "UKinectModule::init(int) initializing object for generic module " << endl;
    OpenNIKinectSingelton::registerModule(this);
}
