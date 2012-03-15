/* 
 * File:   UKinectDepth.cpp
 * Author: lmalek
 * 
 * Created on 15 marzec 2012, 13:13
 */

#include "UKinectDepth.h"
#include "OpenNiKinectSingelton.h"
#include <stdint.h>

using namespace std;
using namespace urbi;

UKinectDepth::UKinectDepth(const std::string& name) : UKinectModule(name) {
    UBindFunction(UKinectDepth, init);
}

UKinectDepth::~UKinectDepth() {
    depthGenerator.Release();
    delete[] depthBufor;
}

void UKinectDepth::init(int) {
    cerr << "UKinectDepth::init()" << endl;
    // Urbi constructor
    mGetNewFrame = true;

    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = depthGenerator.Create(OpenNIKinectSingelton::getInstance().getContext());
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("Failed to initialize camera");

    // Bind all variables
    UBindVar(UKinectDepth, image);
    UBindVar(UKinectDepth, width);
    UBindVar(UKinectDepth, height);
    UBindVar(UKinectDepth, fps);
    UBindVar(UKinectDepth, notify);
    UBindVar(UKinectDepth, flip);

    // Bind all functions
    UBindThreadedFunction(UKinectDepth, getImage, LOCK_INSTANCE);

    // Notify if fps changed
    UNotifyChange(fps, &UKinectDepth::fpsChanged);
    UNotifyChange(notify, &UKinectDepth::changeNotifyImage);

    //obtain image metadata
    depthGenerator.GetMetaData(depthMD);

    // Get image size
    width = depthMD.FullXRes();
    height = depthMD.FullYRes();

    cerr << "\tImage size: x=" << width.as<int>() << " y=" << height.as<int>() << endl;

    UNotifyAccess(image, &UKinectDepth::getImage);

    mBinImage.type = BINARY_IMAGE;
    mBinImage.image.width = width.as<size_t > ();
    mBinImage.image.height = height.as<size_t > ();
    mBinImage.image.imageFormat = IMAGE_GREY8;
    mBinImage.image.size = width.as<size_t > () * height.as<size_t > ();
    mBinImage.image.data = new uint8_t[mBinImage.image.size];
    
    depthBufor = new uint16_t[mBinImage.image.size];

    memset(mBinImage.image.data, 120, mBinImage.image.size);
    memset(depthBufor, 120, mBinImage.image.size*sizeof(uint16_t));

    // Set update period 30
    fps = 30;
    OpenNIKinectSingelton::getInstance().getContext().StartGeneratingAll();
}

void UKinectDepth::getImage() {
    // Lock access to this method from urbi
    boost::lock_guard<boost::mutex> lock(getValMutex);
    XnStatus nRetVal = XN_STATUS_OK;
    // If there is new frame
    if (mGetNewFrame) {
        mGetNewFrame = false;
        nRetVal = OpenNIKinectSingelton::getInstance().getContext().WaitOneUpdateAll(depthGenerator);
        if (nRetVal != XN_STATUS_OK) {
            cerr << "UKinectDepth::getImage() - contex.WaitOneUpdateAll error" << endl;
            return;
        }
        else {
            depthGenerator.GetMetaData(depthMD);
            //memcpy(depthBufor, depthMD.Data(), mBinImage.image.size*sizeof(uint16_t));
            for (int i=0; i < mBinImage.image.size; i++)
                mBinImage.image.data[i] = depthMD[i]/40;
            image = mBinImage;
        }
    }
}

void UKinectDepth::changeNotifyImage(UVar & var) {
    // Always unnotify
    image.unnotify();
    if (var.as<bool>())
        UNotifyAccess(image, &UKinectDepth::getImage);
}

int UKinectDepth::update() {
    mGetNewFrame = true;
    return 0;
}

void UKinectDepth::fpsChanged() {
    cerr << "UKinectDepth::fpsChanged()" << endl
            << "\tCamera fps changed to " << fps.as<int>() << endl;
    USetUpdate(fps.as<int>() > 0 ? 1000.0 / fps.as<int>() : -1.0);
}

UStart(UKinectDepth);