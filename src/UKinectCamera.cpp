/* 
 * File:   uopennikinect.cpp
 * Author: lmalek
 * 
 * Created on 7 marzec 2012, 15:39
 */
#include "UKinectCamera.h"
#include "OpenNiKinectSingelton.h"
#include <stdint.h>

using namespace std;
using namespace urbi;

UKinectCamera::UKinectCamera(const std::string& name) : UKinectModule(name) {
    UBindFunction(UKinectCamera, init);
}

UKinectCamera::~UKinectCamera() {
    imageGenerator.Release();
}

void UKinectCamera::init() {
    cerr << "UKinectCamera::init()" << endl;
    // Urbi constructor
    mGetNewFrame = true;

    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = imageGenerator.Create(OpenNIKinectSingelton::getInstance().getContext());
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("Failed to initialize camera");

    // Bind all variables
    UBindVar(UKinectCamera, image);
    UBindVar(UKinectCamera, width);
    UBindVar(UKinectCamera, height);
    UBindVar(UKinectCamera, fps);
    UBindVar(UKinectcamera, notify);
    UBindVar(UKinectCamera, flip);

    // Bind all functions
    UBindThreadedFunction(UKinectCamera, getData, LOCK_INSTANCE);
    
    // Notify if fps changed
    UNotifyChange(fps, &UKinectCamera::fpsChanged);
    UNotifyChange(notify, &UKinectCamera::changeNotifyImage);

    //obtain image metadata
    imageGenerator.GetMetaData(imageMD);
    imageGenerator.SetPixelFormat(XN_PIXEL_FORMAT_RGB24);

    // Get image size
    width = imageMD.FullXRes();
    height = imageMD.FullYRes();

    cerr << "\tImage size: x=" << width.as<int>() << " y=" << height.as<int>() << endl;

    //UNotifyAccess(image, &UKinectCamera::getData);

    mBinImage.type = BINARY_IMAGE;
    mBinImage.image.width = width.as<size_t > ();
    mBinImage.image.height = height.as<size_t > ();
    mBinImage.image.imageFormat = IMAGE_RGB;
    mBinImage.image.size = width.as<size_t > () * height.as<size_t > () * 3;
    mBinImage.image.data = new uint8_t[mBinImage.image.size];

    memset(mBinImage.image.data, 120, mBinImage.image.size * sizeof(uint8_t));

    // Set update period 30
    fps = 30;
    OpenNIKinectSingelton::getInstance().getContext().StartGeneratingAll();
}

/* 
 * mode = 0 - slave mode in multi generator work
 * mode = 1 - master mode in multi generator work
 * mode = 2 - single mode
 */
void UKinectCamera::getData(int mode) {
    // Lock access to this method from urbi
    boost::lock_guard<boost::mutex> lock(getValMutex);
    XnStatus nRetVal = XN_STATUS_OK;
    // If there is new frame
    if (mGetNewFrame) {
        mGetNewFrame = false;
        if (mode == 1) {
            nRetVal = OpenNIKinectSingelton::getInstance().getContext().WaitAndUpdateAll();
        }
        if (mode == 2) {
            nRetVal = OpenNIKinectSingelton::getInstance().getContext().WaitOneUpdateAll(imageGenerator);
        }        
        if (nRetVal != XN_STATUS_OK) {
            cerr << "UKinectCamera::getData() - contex.WaitOneUpdateAll error" << endl;
            return;
        }
        else {
            imageGenerator.GetMetaData(imageMD);
            memcpy(mBinImage.image.data, imageMD.Data(), mBinImage.image.size);
            image = mBinImage;
        }
    }
}

void UKinectCamera::changeNotifyImage(UVar & var) {
    // Always unnotify
    image.unnotify();
    if (var.as<bool>())
        UNotifyAccess(image, &UKinectCamera::getData);
}

int UKinectCamera::update() {
    mGetNewFrame = true;
    return 0;
}

void UKinectCamera::fpsChanged() {
    if (fps.as<int>() > imageMD.FPS())
        fps = imageMD.FPS();
    cerr << "UKinectCamera::fpsChanged()" << endl
            << "\tCamera fps changed to " << fps.as<int>() << endl;
    USetUpdate(fps.as<int>() > 0 ? 1000.0 / fps.as<int>() : -1.0);
}

UStart(UKinectCamera);