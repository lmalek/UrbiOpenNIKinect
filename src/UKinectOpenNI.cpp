/* 
 * File:   UKinectOpenNI.cpp
 * Author: lmalek
 * 
 * Created on 21 marzec 2012, 09:55
 */

#include "UKinectOpenNI.h"

using namespace std;
using namespace urbi;

#define CHECK_RC(nRetVal, what)					    \
    if (nRetVal != XN_STATUS_OK)				    \
{								    \
    printf("%s failed: %s\n", what, xnGetStatusString(nRetVal));    \
    return;						    \
}

XnBool g_bNeedPose = FALSE;
XnChar g_strPose[20] = "";
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie);
void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie);

UKinectOpenNI::UKinectOpenNI(const std::string& name) : UObject(name) {
    cerr << "[UKinectOpenNI] loaded" << endl;
    UBindFunction(UKinectOpenNI, init);
}

UKinectOpenNI::~UKinectOpenNI() {
    if (imageActive)
        deactivateImage();
    if (depthActive)
        deactivateDepth();
    if (userActive)
        deactivateUsers();
    context.Release();
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
    UBindVar(UKinectOpenNI, imageWidth);
    UBindVar(UKinectOpenNI, imageHeight);
    UBindVar(UKinectOpenNI, depth);
    UBindVar(UKinectOpenNI, depthWidth);
    UBindVar(UKinectOpenNI, depthHeight);
    UBindVar(UKinectOpenNI, numUsers);
    UBindVar(UKinectOpenNI, fps);
    UBindVar(UKinectOpenNI, notify);

    // Bind all functions
    UBindThreadedFunction(UKinectOpenNI, refreshData, LOCK_INSTANCE);
    UBindFunction(UKinectOpenNI, getImage);
    UBindFunction(UKinectOpenNI, getDepth);
    UBindFunction(UKinectOpenNI, getUsers);
    
    UBindFunction(UKinectOpenNI, getJointPosition);
    UBindFunction(UKinectOpenNI, getDepthXY);
    UBindFunction(UKinectOpenNI, getDepthMedianFromArea);

    // Notify if fps changed
    UNotifyChange(fps, &UKinectOpenNI::fpsChanged);
    //UNotifyChange(image, &UKinectOpenNI::getImage);
    //UNotifyChange(depth, &UKinectOpenNI::getDepth);

    imageActive = false;
    depthActive = false;
    userActive = false;

    if (imageFlag)
        activateImage();
    if (depthFlag)
        activateDepth();
    if (userFlag)
        activateUsers();

    // Set update period 30
    fps = 30;
    context.StartGeneratingAll();
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

    cerr << "\t[UKinectOpenNI] Image size: x=" << imageWidth.as<int>() << " y=" << imageHeight.as<int>() << endl;

    mBinImage.type = BINARY_IMAGE;
    mBinImage.image.width = imageWidth.as<size_t > ();
    mBinImage.image.height = imageHeight.as<size_t > ();
    mBinImage.image.imageFormat = IMAGE_RGB;
    mBinImage.image.size = imageWidth.as<size_t > () * imageHeight.as<size_t > () * 3;
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

    
        XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        throw runtime_error("Supplied user generator doesn't support skeleton");
    }
    nRetVal = userGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, &userGenerator, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = userGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, &userGenerator, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, &userGenerator, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (userGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        g_bNeedPose = TRUE;
        if (!userGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            throw runtime_error("Pose required, but not supported");
        }
        nRetVal = userGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, &userGenerator, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        userGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }
    
    userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);
    
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
        nRetVal = context.WaitAndUpdateAll();
        if (nRetVal != XN_STATUS_OK) {
            cerr << "[UKinectOpenNI]::getData() - contex.WaitAndUpdateAll error" << endl;
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
    ;
    depth = mBinDepth;
}

unsigned int UKinectOpenNI::getDepthXY(unsigned int x, unsigned int y) {
    return depthMD(x, y);
}

unsigned int UKinectOpenNI::getDepthMedianFromArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
    vector<uint16_t> area;
    uint16_t middle;
    if (x2 > depthWidth.as<int>())
        x2 = depthWidth.as<int>();
    if (y2 > depthHeight.as<int>())
        y2 = depthHeight.as<int>();
    for (uint16_t x = x1; x <= x2; x++)
        for (uint16_t y = y1; y <= y2; y++)
            area.push_back(depthMD(x, y));
    sort(area.begin(), area.begin());
    middle = floor(area.size()/2);
    return area[middle];
}

void UKinectOpenNI::getUsers() {
    nUsers = MAX_NUM_USERS;
    unsigned int userCount=0;
    userGenerator.GetUsers(aUsers, nUsers);
    for (XnUInt16 i = 0; i < nUsers; i++) {
        if (userGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
            userCount++;
    }
    numUsers = userCount;
}

std::vector<float> UKinectOpenNI::setVectorPosition(unsigned int user, XnSkeletonJoint eJoint) {
    std::vector<float> position;
    XnSkeletonJointPosition joint;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(user, eJoint, joint);
    position.push_back(joint.position.X);
    position.push_back(joint.position.Y);
    position.push_back(joint.position.Z);
    return position;
}

std::vector<float> UKinectOpenNI::getJointPosition(unsigned int user, unsigned int jointNumber) {
    std::vector<float> position;
    if (user>numUsers.as<int>()) {
        cerr<<"[UKinectOpenNI] - wrong user number"<<endl;
        return position;
    }
    // find user ID
    if (userGenerator.GetSkeletonCap().IsTracking(user) == FALSE){
        cerr<<"[UKinectOpenNI] - user not tracked"<<endl;
        return position;
    }
    if (jointNumber>24)
        return position;
    switch (jointNumber) {
        case 1:
            return setVectorPosition(user, XN_SKEL_HEAD);
        case 2:
            return setVectorPosition(user, XN_SKEL_NECK);
        case 3:
            return setVectorPosition(user, XN_SKEL_TORSO);
        case 4:
            return setVectorPosition(user, XN_SKEL_WAIST);
        case 5:
            return setVectorPosition(user, XN_SKEL_LEFT_COLLAR);         
        case 6:
            return setVectorPosition(user, XN_SKEL_LEFT_SHOULDER);            
        case 7:
            return setVectorPosition(user, XN_SKEL_LEFT_ELBOW);        
        case 8:
            return setVectorPosition(user, XN_SKEL_LEFT_WRIST);            
        case 9:
            return setVectorPosition(user, XN_SKEL_LEFT_HAND);           
        case 10:
            return setVectorPosition(user, XN_SKEL_LEFT_FINGERTIP);           
        case 11:
            return setVectorPosition(user, XN_SKEL_RIGHT_COLLAR);          
        case 12:
            return setVectorPosition(user, XN_SKEL_RIGHT_SHOULDER);         
        case 13:
            return setVectorPosition(user, XN_SKEL_RIGHT_ELBOW);           
        case 14:
            return setVectorPosition(user, XN_SKEL_RIGHT_WRIST);           
        case 15:
            return setVectorPosition(user, XN_SKEL_RIGHT_HAND);          
        case 16:
            return setVectorPosition(user, XN_SKEL_RIGHT_FINGERTIP);         
        case 17:
            return setVectorPosition(user, XN_SKEL_LEFT_HIP);          
        case 18:
            return setVectorPosition(user, XN_SKEL_LEFT_KNEE);            
        case 19:
            return setVectorPosition(user, XN_SKEL_LEFT_ANKLE);           
        case 20:
            return setVectorPosition(user, XN_SKEL_LEFT_FOOT);           
        case 21:
            return setVectorPosition(user, XN_SKEL_RIGHT_HIP);     
        case 22:
            return setVectorPosition(user, XN_SKEL_RIGHT_KNEE);           
        case 23:
            return setVectorPosition(user, XN_SKEL_RIGHT_ANKLE);            
        case 24:
            return setVectorPosition(user, XN_SKEL_RIGHT_FOOT);                               
    }
    return position;
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

void UKinectOpenNI::fpsChanged() {
    cerr << "UKinectOpenNI::fpsChanged()" << endl
            << "\tCamera fps changed to " << fps.as<int>() << endl;
    USetUpdate(fps.as<int>() > 0 ? 1000.0 / fps.as<int>() : -1.0);
}

/*
 *
 * 
 */

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*>(pCookie);
    printf("New User %d\n", nId);
    // New user found
    if (g_bNeedPose)
    {
        g_UserGenerator->GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    }
    else
    {
        g_UserGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}
// Callback: An existing user was lost
void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    printf("Lost user %d\n", nId);	
}
// Callback: Detected a pose
void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie)
{
    xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*>(pCookie);
    printf("Pose %s detected for user %d\n", strPose, nId);
    g_UserGenerator->GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
    printf("Calibration started for user %d\n", nId);
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
    xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*>(pCookie);
    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        printf("Calibration complete, start tracking user %d\n", nId);		
        g_UserGenerator->GetSkeletonCap().StartTracking(nId);
    }
    else
    {
        // Calibration failed
        printf("Calibration failed for user %d\n", nId);
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
        {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
        if (g_bNeedPose)
        {
            g_UserGenerator->GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        }
        else
        {
            g_UserGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}



UStart(UKinectOpenNI);