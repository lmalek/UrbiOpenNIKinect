/*
 * Kinect module based on OpenNI for URBI
 * Copyright (C) 2012  Lukasz Malek
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
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
    cerr << "[UKinectOpenNI]::UKinectOpenNI()" << endl;
    UBindFunction(UKinectOpenNI, init);
}

UKinectOpenNI::~UKinectOpenNI() {
    cerr << "[UKinectOpenNI]::~UKinectOpenNI()" << endl;
    if (imageActive)
        deactivateImage();
    if (depthActive)
        deactivateDepth();
    if (usersActive)
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
        ss << "[UKinectOpenNI]::init() : Failed to initialize kinect context: " << xnGetStatusString(nRetVal);
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
    UBindVar(UKinectOpenNI, skeleton);
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
    UBindFunction(UKinectOpenNI, matchDepthToImage);

    UBindFunction(UKinectOpenNI, getSkeleton);

    // Notify if fps changed
    UNotifyChange(fps, &UKinectOpenNI::fpsChanged);
    //UNotifyChange(image, &UKinectOpenNI::getImage);
    //UNotifyChange(depth, &UKinectOpenNI::getDepth);

    imageActive = false;
    depthActive = false;
    usersActive = false;

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
        throw runtime_error("[UKinectOpenNI]::activateImage() : Failed to initialize image");

    //obtain image metadata
    imageGenerator.GetMetaData(imageMD);

    // Get image size
    imageWidth = imageMD.FullXRes();
    imageHeight = imageMD.FullYRes();

    cerr << "\t[UKinectOpenNI]::activateImage() : Image size: x=" << imageWidth.as<int>() << " y=" << imageHeight.as<int>() << endl;

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
    delete[] mBinImage.image.data;
    mBinImage.image.data = NULL;
    imageGenerator.Release();
}

void UKinectOpenNI::activateDepth() {
    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = depthGenerator.Create(context);
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("[UKinectOpenNI]::activateDepth() : Failed to initialize depth");

    //obtain image metadata
    depthGenerator.GetMetaData(depthMD);

    // Get image size
    depthWidth = depthMD.FullXRes();
    depthHeight = depthMD.FullYRes();

    cerr << "\t[UKinectOpenNI]::activateDepth() : Depth size: x=" << depthWidth.as<int>() << " y=" << depthHeight.as<int>() << endl;

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
    delete[] mBinDepth.image.data;
    mBinDepth.image.data = NULL;
    depthGenerator.Release();
}

void UKinectOpenNI::activateUsers() {
    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = userGenerator.Create(context);
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("[UKinectOpenNI]::activateUsers() : Failed to initialize user");


    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!userGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON)) {
        throw runtime_error("Supplied user generator doesn't support skeleton");
    }
    nRetVal = userGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, &userGenerator, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = userGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, &userGenerator, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = userGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, &userGenerator, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (userGenerator.GetSkeletonCap().NeedPoseForCalibration()) {
        g_bNeedPose = TRUE;
        if (!userGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)) {
            throw runtime_error("Pose required, but not supported");
        }
        nRetVal = userGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, &userGenerator, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        userGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    userGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    usersActive = true;
}

void UKinectOpenNI::deactivateUsers() {
    usersActive = false;
    delete[] mBinSkeleton.image.data;
    mBinSkeleton.image.data = NULL;
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
            cerr << "[UKinectOpenNI]::refreshData() : contex.WaitAndUpdateAll error" << endl;
            return;
        }
    }
}

void UKinectOpenNI::getImage() {
    if (!imageActive) return;
    imageGenerator.GetMetaData(imageMD);
    memcpy(mBinImage.image.data, imageMD.Data(), mBinImage.image.size);
    image = mBinImage;
}

void UKinectOpenNI::getDepth() {
    if (!depthActive) return;
    depthGenerator.GetMetaData(depthMD);
    for (int i = 0; i < mBinDepth.image.size; i++)
        mBinDepth.image.data[i] = depthMD[i] / 40;
    ;
    depth = mBinDepth;
}

unsigned int UKinectOpenNI::getDepthXY(unsigned int x, unsigned int y) {
    if (!depthActive) return 0;
    return depthMD(x, y);
}

unsigned int UKinectOpenNI::getDepthMedianFromArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
    if (!depthActive) return 0;
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
    middle = floor(area.size() / 2);
    return area[middle];
}

void UKinectOpenNI::matchDepthToImage(bool state) {
    if (!depthActive) return;
    XnStatus nRetVal = XN_STATUS_OK;
    if (state) {
        nRetVal = depthGenerator.GetAlternativeViewPointCap().SetViewPoint(imageGenerator);
        if(nRetVal)
           cerr<< "Failed to match Depth and RGB points of view: " << xnGetStatusString(nRetVal) << endl;
    }
    else {
        nRetVal = depthGenerator.GetAlternativeViewPointCap().ResetViewPoint();
    }
}

void UKinectOpenNI::getUsers() {
    if (!usersActive) return;
    nUsers = MAX_NUM_USERS;
    unsigned int userCount = 0;
    userGenerator.GetUsers(aUsers, nUsers);
    for (XnUInt16 i = 0; i < nUsers; i++) {
        if (userGenerator.GetSkeletonCap().IsTracking(aUsers[i]))
            userCount++;
    }
    numUsers = userCount;
}

std::vector<float> UKinectOpenNI::getJointPosition(unsigned int user, unsigned int jointNumber) {
    std::vector<float> position;
    XnSkeletonJointPosition joint;
    XnSkeletonJoint eJoint;
    if (user > numUsers.as<int>()) {
        cerr << "[UKinectOpenNI]::getJointPosition() : - wrong user number" << endl;
        return position;
    }
    // find user ID
    if (userGenerator.GetSkeletonCap().IsTracking(user) == FALSE) {
        cerr << "[UKinectOpenNI]::getJointPosition() : user not tracked" << endl;
        return position;
    }
    try {
        eJoint = jointNumberToSkeleton(jointNumber);
    }
    catch (std::range_error) {
        return position;
    }
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(user, eJoint, joint);
    position.push_back(joint.position.X);
    position.push_back(joint.position.Y);
    position.push_back(joint.position.Z);
    return position;
}

std::vector<int> UKinectOpenNI::getJointImageCoordinate(unsigned int user, unsigned int jointNumber) {
    XnSkeletonJointPosition joint;
    XnSkeletonJoint eJoint;
    std::vector<int> coordinate;
    if (user > numUsers.as<int>()) {
        cerr << "[UKinectOpenNI]::getJointPosition() : - wrong user number" << endl;
        return coordinate;
    }
    // find user ID
    if (userGenerator.GetSkeletonCap().IsTracking(user) == FALSE) {
        cerr << "[UKinectOpenNI]::getJointPosition() : user not tracked" << endl;
        return coordinate;
    }
    try {
        eJoint = jointNumberToSkeleton(jointNumber);
    }
    catch (std::range_error) {
        return coordinate;
    }
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(user, eJoint, joint);
    XnPoint3D pt = joint.position;
    coordinate.push_back(pt.X);
    coordinate.push_back(pt.Y);
    return coordinate;
}

XnSkeletonJoint UKinectOpenNI::jointNumberToSkeleton(unsigned int jointNumber) {
    if (jointNumber > 24)
        throw new std::range_error("Exeption joint out of range");
    return static_cast<XnSkeletonJoint>(jointNumber);
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

void UKinectOpenNI::DrawLimb(cv::Mat& processImage, XnUserID player, XnSkeletonJoint eJoint1, XnSkeletonJoint eJoint2) {
    if (!depthActive) {
        printf("[UKinectOpenNI]::DrawLimb() : depth required!\n");
        return;
    }

    if (!userGenerator.GetSkeletonCap().IsTracking(player)) {
        return;
    }

    XnSkeletonJointPosition joint1, joint2;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint1, joint1);
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(player, eJoint2, joint2);

    if (joint1.fConfidence < 0.5 || joint2.fConfidence < 0.5) {
        return;
    }

    XnPoint3D pt[2];
    pt[0] = joint1.position;
    pt[1] = joint2.position;

    depthGenerator.ConvertRealWorldToProjective(2, pt, pt);
    line(processImage, cv::Point(pt[0].X, pt[0].Y),
            cv::Point(pt[1].X, pt[1].Y), cv::Scalar(255, 0, 0), 2);
}

void UKinectOpenNI::getSkeleton(UImage src) {
    if (!depthActive) {
        printf("[UKinectOpenNI]::getSkeleton() depth required!\n");
        return;
    }
    delete[] mBinSkeleton.image.data;
    mBinSkeleton.image.data = NULL;
    
    int format;
    if (src.imageFormat == IMAGE_RGB)
        format = CV_8UC3;
    else if (src.imageFormat == IMAGE_GREY8)
        format = CV_8UC1;
    else
        skeleton = src;
    cv::Mat processImage(cv::Size(src.width, src.height), format, src.data);    

    for (int i = 0; i < nUsers; ++i) {
        if (!userGenerator.GetSkeletonCap().IsTracking(aUsers[i])) {
            continue;
        }
        DrawLimb(processImage, aUsers[i], XN_SKEL_HEAD, XN_SKEL_NECK);

        DrawLimb(processImage, aUsers[i], XN_SKEL_NECK, XN_SKEL_LEFT_SHOULDER);
        DrawLimb(processImage, aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_LEFT_ELBOW);
        DrawLimb(processImage, aUsers[i], XN_SKEL_LEFT_ELBOW, XN_SKEL_LEFT_HAND);

        DrawLimb(processImage, aUsers[i], XN_SKEL_NECK, XN_SKEL_RIGHT_SHOULDER);
        DrawLimb(processImage, aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_RIGHT_ELBOW);
        DrawLimb(processImage, aUsers[i], XN_SKEL_RIGHT_ELBOW, XN_SKEL_RIGHT_HAND);

        DrawLimb(processImage, aUsers[i], XN_SKEL_LEFT_SHOULDER, XN_SKEL_TORSO);
        DrawLimb(processImage, aUsers[i], XN_SKEL_RIGHT_SHOULDER, XN_SKEL_TORSO);

        DrawLimb(processImage, aUsers[i], XN_SKEL_TORSO, XN_SKEL_LEFT_HIP);
        DrawLimb(processImage, aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_LEFT_KNEE);
        DrawLimb(processImage, aUsers[i], XN_SKEL_LEFT_KNEE, XN_SKEL_LEFT_FOOT);

        DrawLimb(processImage, aUsers[i], XN_SKEL_TORSO, XN_SKEL_RIGHT_HIP);
        DrawLimb(processImage, aUsers[i], XN_SKEL_RIGHT_HIP, XN_SKEL_RIGHT_KNEE);
        DrawLimb(processImage, aUsers[i], XN_SKEL_RIGHT_KNEE, XN_SKEL_RIGHT_FOOT);

        DrawLimb(processImage, aUsers[i], XN_SKEL_LEFT_HIP, XN_SKEL_RIGHT_HIP);
    }
    
    mBinSkeleton.type = BINARY_IMAGE;
    mBinSkeleton.image.width = processImage.cols;
    mBinSkeleton.image.height = processImage.rows;
    mBinSkeleton.image.size = src.size;
    mBinSkeleton.image.data = new uint8_t[mBinSkeleton.image.size];
    mempcpy(mBinSkeleton.image.data, processImage.data, mBinSkeleton.image.size);
    mBinSkeleton.image.imageFormat = src.imageFormat;
    skeleton = mBinSkeleton;
}

/*
 *
 * 
 */

// Callback: New user was detected

void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
    xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*> (pCookie);
    printf("New User %d\n", nId);
    // New user found
    if (g_bNeedPose) {
        g_UserGenerator->GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    } else {
        g_UserGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
}
// Callback: An existing user was lost

void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie) {
    printf("Lost user %d\n", nId);
}
// Callback: Detected a pose

void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& capability, const XnChar* strPose, XnUserID nId, void* pCookie) {
    xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*> (pCookie);
    printf("Pose %s detected for user %d\n", strPose, nId);
    g_UserGenerator->GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration

void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie) {
    printf("Calibration started for user %d\n", nId);
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie) {
    xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*> (pCookie);
    if (eStatus == XN_CALIBRATION_STATUS_OK) {
        // Calibration succeeded
        printf("Calibration complete, start tracking user %d\n", nId);
        g_UserGenerator->GetSkeletonCap().StartTracking(nId);
    } else {
        // Calibration failed
        printf("Calibration failed for user %d\n", nId);
        if (eStatus == XN_CALIBRATION_STATUS_MANUAL_ABORT) {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
        if (g_bNeedPose) {
            g_UserGenerator->GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        } else {
            g_UserGenerator->GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}



UStart(UKinectOpenNI);