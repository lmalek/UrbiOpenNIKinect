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
}

void UKinectDepth::init() {
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
    UBindThreadedFunction(UKinectDepth, getData, LOCK_INSTANCE);
    UBindFunction(UKinectDepth, getXY);
    UBindFunction(UKinectDepth, getMedianFromArea);

    // Notify if fps changed
    UNotifyChange(fps, &UKinectDepth::fpsChanged);
    UNotifyChange(notify, &UKinectDepth::changeNotifyImage);

    //obtain image metadata
    depthGenerator.GetMetaData(depthMD);

    // Get image size
    width = depthMD.FullXRes();
    height = depthMD.FullYRes();

    cerr << "\tImage size: x=" << width.as<int>() << " y=" << height.as<int>() << endl;

    //UNotifyAccess(image, &UKinectDepth::getData);

    mBinImage.type = BINARY_IMAGE;
    mBinImage.image.width = width.as<size_t > ();
    mBinImage.image.height = height.as<size_t > ();
    mBinImage.image.imageFormat = IMAGE_GREY8;
    mBinImage.image.size = width.as<size_t > () * height.as<size_t > ();
    mBinImage.image.data = new uint8_t[mBinImage.image.size];

    memset(mBinImage.image.data, 120, mBinImage.image.size);

    // Set update period 30
    fps = 30;
    OpenNIKinectSingelton::getInstance().getContext().StartGeneratingAll();
}

/* 
 * mode = 0 - slave mode in multi generator work
 * mode = 1 - master mode in multi generator work
 * mode = 2 - single mode
 * depth resolution is 1 per 40 mm
 */
void UKinectDepth::getData(int mode) {
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
            nRetVal = OpenNIKinectSingelton::getInstance().getContext().WaitOneUpdateAll(depthGenerator);
        }
        if (nRetVal != XN_STATUS_OK) {
            cerr << "UKinectDepth::getData() - contex.WaitOneUpdateAll error" << endl;
            return;
        } else {
            depthGenerator.GetMetaData(depthMD);
            for (int i = 0; i < mBinImage.image.size; i++)
                mBinImage.image.data[i] = depthMD[i] / 40;
            image = mBinImage;
        }
    }
}

unsigned int UKinectDepth::getXY(unsigned int x, unsigned int y) {
    return depthMD(x, y);
}

unsigned int UKinectDepth::getMedianFromArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
    vector<uint16_t> area;
    uint16_t middle;
    if (x2 > width.as<int>())
        x2 = width.as<int>();
    if (y2 > height.as<int>())
        y2 = height.as<int>();
    for (uint16_t x = x1; x <= x2; x++)
        for (uint16_t y = y1; y <= y2; y++)
            area.push_back(depthMD(x, y));
    sort(area.begin(), area.begin());
    middle = floor(area.size()/2);
    return area[middle];
}

void UKinectDepth::changeNotifyImage(UVar & var) {
    // Always unnotify
    image.unnotify();
    if (var.as<bool>())
        UNotifyAccess(image, &UKinectDepth::getData);
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