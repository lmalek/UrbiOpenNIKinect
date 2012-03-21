/* 
 * File:   UKinectOpenNi.cpp
 * Author: lmalek
 * 
 * Created on 21 marzec 2012, 09:55
 */

#include "UKinectOpenNI.h"

using namespace std;
using namespace urbi;

UKinectOpenNI::UKinectOpenNI(const std::string& name) : UObject(name) {
    UBindFunction(UKinectOpenNi, init);
}

UKinectOpenNI::~UKinectOpenNI() {
}

int UKinectOpenNI::update() {
    mGetNewData = true;
    return 0;
}

void UKinectOpenNI::init(bool imageFlag, bool depthFlag, bool userFlag) {
    cerr << "[UKinectOpenNI]::init()" << endl;
    // Urbi constructor
    mGetNewData = true;

    XnStatus nRetVal = XN_STATUS_OK;

    nRetVal = context.Init();
    if (nRetVal != XN_STATUS_OK) { // failed to initializew
        std::stringstream ss;
        ss << "[UKinectOpenNI] Failed to initialize kinect context: " << xnGetStatusString(nRetVal);
        throw std::runtime_error(ss.str());
    }

    // Bind all variables
    UBindVar(UKinectOpenNI, image);
    UBindVar(UKinectOpenNI, width);
    UBindVar(UKinectOpenNI, height);
    UBindVar(UKinectOpenNI, fps);
    UBindVar(UKinectOpenNI, notify);
    UBindVar(UKinectOpenNI, flip);

    // Bind all functions
    UBindThreadedFunction(UKinectOpenNI, refreshData, LOCK_INSTANCE);
    UBindFunction(UKinectOpenNI, getXY);
    UBindFunction(UKinectOpenNI, getMedianFromArea);

    // Notify if fps changed
    UNotifyChange(fps, &UKinectOpenNI::fpsChanged);

    imageActive = false;
    depthActive = false;
    userActive = false;

    if (imageFlag)
        activateImage();
    if (depthFlag)
        activateDepth();
    if (userFlag)
        activateUser();

    // Set update period 30
    fps = 30;
    OpenNIKinectSingelton::getInstance().getContext().StartGeneratingAll();
}

void UKinectOpenNI::activateImage() {
    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = imageGenerator.Create(context);
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("[UKinectOpenNI] Failed to initialize image");

    //obtain image metadata
    imageGenerator.GetMetaData(imageMD);

    // Get image size
    imageWidth = imageMD.FullXRes();
    imageHeight = imageMD.FullYRes();

    cerr << "\t[UKinectOpenNI] Image size: x=" << depthWidth.as<int>() << " y=" << depthHeight.as<int>() << endl;

    mBinImage.type = BINARY_IMAGE;
    mBinImage.image.width = depthWidth.as<size_t > ();
    mBinImage.image.height = depthHeight.as<size_t > ();
    mBinImage.image.imageFormat = IMAGE_RGB;
    mBinImage.image.size = depthWidth.as<size_t > () * depthHeight.as<size_t > () * 3;
    mBinImage.image.data = new uint8_t[mBinImage.image.size];

    memset(mBinImage.image.data, 120, mBinImage.image.size * sizeof (uint8_t));

    imageActive = true;
}

void UKinectOpenNI::deactivateImage() {
    imageActive = false;
    imageGenerator.Release();
}

void UKinectOpenNI::activateDepth() {
    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = depthGenerator.Create(context);
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("[UKinectOpenNI] Failed to initialize depth");

    //obtain image metadata
    depthGenerator.GetMetaData(depthMD);

    // Get image size
    depthWidth = depthMD.FullXRes();
    depthHeight = depthMD.FullYRes();

    cerr << "\t[UKinectOpenNI] Depth size: x=" << depthWidth.as<int>() << " y=" << depthHeight.as<int>() << endl;

    mBinDepth.type = BINARY_IMAGE;
    mBinDepth.image.width = depthWidth.as<size_t > ();
    mBinDepth.image.height = depthHeight.as<size_t > ();
    mBinDepth.image.imageFormat = IMAGE_GREY8;
    mBinDepth.image.size = depthWidth.as<size_t > () * depthHeight.as<size_t > ();
    mBinDepth.image.data = new uint8_t[mBinDepth.image.size];

    memset(mBinDepth.image.data, 120, mBinDepth.image.size);

    depthActive = true;
}

void UKinectOpenNI::deactivateDepth() {
    depthActive = false;
    depthGenerator.Release();
}

void UKinectOpenNI::activateUsers() {
    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = userGenerator.Create(context);
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("[UKinectOpenNI] Failed to initialize user");

    userActive = true;
}

void UKinectOpenNI::deactivateUsers() {
    userActive = false;
    userGenerator.Release();
}

void UKinectOpenNI::refreshData() {
    // Lock access to this method from urbi
    boost::lock_guard<boost::mutex> lock(getValMutex);
    XnStatus nRetVal = XN_STATUS_OK;
    // If there is new frame
    if (mGetNewData) {
        mGetNewData = false;
        nRetVal = OpenNIKinectSingelton::getInstance().getContext().WaitAndUpdateAll();
        if (nRetVal != XN_STATUS_OK) {
            cerr << "UKinectDepth::getData() - contex.WaitAndUpdateAll error" << endl;
            return;
        }
    }
}

void UKinectOpenNI::getImage() {
    imageGenerator.GetMetaData(imageMD);
    memcpy(mBinImage.image.data, imageMD.Data(), mBinImage.image.size);
    image = mBinImage;
}

void UKinectOpenNI::getDepth() {
    depthGenerator.GetMetaData(depthMD);
    for (int i = 0; i < mBinDepth.image.size; i++)
        mBinDepth.image.data[i] = depthMD[i] / 40;
    depth = mBinImage;
}

#define MAX_NUM_USERS 16
void UKinectOpenNI::getUsers() {
    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    nUsers=MAX_NUM_USERS;
    g_UserGenerator.GetUsers(aUsers, nUsers);
    
    imageGenerator.GetMetaData(imageMD);
    memcpy(mBinImage.image.data, imageMD.Data(), mBinImage.image.size);
    image = mBinImage;
}

void UKinectOpenNI::changeNotifyImage(UVar & var) {
    // Always unnotify
    image.unnotify();
    if (var.as<bool>())
        UNotifyAccess(image, &UKinectOpenNI::getImage);
}

void UKinectOpenNI::changeNotifyDepth(UVar & var) {
    // Always unnotify
    depth.unnotify();
    if (var.as<bool>())
        UNotifyAccess(depth, &UKinectOpenNI::getDepth);
}

void UKinectOpenNI::changeNotifyUsers(UVar & var) {
    // Always unnotify
    numUsers.unnotify();
    if (var.as<bool>())
        UNotifyAccess(numUsers, &UKinectOpenNI::getUsers);
}