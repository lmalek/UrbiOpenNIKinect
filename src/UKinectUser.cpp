/* 
 * File:   UKinectUser.cpp
 * Author: lmalek
 * 
 * Created on 19 marzec 2012, 11:13
 */

#include "UKinectUser.h"
#include "OpenNiKinectSingelton.h"
#include <stdint.h>


#include <boost/thread.hpp>
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
xn::Context g_Context;
xn::ScriptNode g_scriptNode;
xn::UserGenerator g_UserGenerator;

boost::thread mainThread;

#define SAMPLE_XML_PATH "/home/lmalek/Pulpit/OpenNI/OpenNI-Bin-Dev-Linux-x64-v1.5.2.23/Samples/Config/SamplesConfig.xml"
#define SAMPLE_XML_PATH_LOCAL "SamplesConfig.xml"

XnBool fileExists(const char *fn)
{
	XnBool exists;
	xnOSDoesFileExist(fn, &exists);
	return exists;
}

// Callback: New user was detected
void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& generator, XnUserID nId, void* pCookie)
{
    //xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*>(pCookie);
    printf("New User %d\n", nId);
    // New user found
    if (g_bNeedPose)
    {
        g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
    }
    else
    {
        g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
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
   // xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*>(pCookie);
    printf("Pose %s detected for user %d\n", strPose, nId);
    g_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}
// Callback: Started calibration
void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nId, void* pCookie)
{
    printf("Calibration started for user %d\n", nId);
}

void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& capability, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
    //xn::UserGenerator* g_UserGenerator = static_cast<xn::UserGenerator*>(pCookie);
    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        printf("Calibration complete, start tracking user %d\n", nId);		
        g_UserGenerator.GetSkeletonCap().StartTracking(nId);
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
            g_UserGenerator.GetPoseDetectionCap().StartPoseDetection(g_strPose, nId);
        }
        else
        {
            g_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}



void UKinectUser::mainThreadFunction() {
    XnStatus nRetVal = XN_STATUS_OK;
    xn::EnumerationErrors errors;

    const char *fn = NULL;
    if    (fileExists(SAMPLE_XML_PATH)) fn = SAMPLE_XML_PATH;
    else if (fileExists(SAMPLE_XML_PATH_LOCAL)) fn = SAMPLE_XML_PATH_LOCAL;
    else {
        printf("Could not find '%s' nor '%s'. Aborting.\n" , SAMPLE_XML_PATH, SAMPLE_XML_PATH_LOCAL);
//        return XN_STATUS_ERROR;
    }
    printf("Reading config from: '%s'\n", fn);

    nRetVal = g_Context.InitFromXmlFile(fn, g_scriptNode, &errors);
    if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
    {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
//        return (nRetVal);
    }
    else if (nRetVal != XN_STATUS_OK)
    {
        printf("Open failed: %s\n", xnGetStatusString(nRetVal));
//        return (nRetVal);
    }

    CHECK_RC(nRetVal,"No depth");

    nRetVal = g_Context.FindExistingNode(XN_NODE_TYPE_USER, g_UserGenerator);
    if (nRetVal != XN_STATUS_OK)
    {
        nRetVal = g_UserGenerator.Create(g_Context);
        CHECK_RC(nRetVal, "Find user generator");
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
//        return 1;
    }
    nRetVal = g_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(nRetVal, "Register to user callbacks");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(nRetVal, "Register to calibration start");
    nRetVal = g_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(nRetVal, "Register to calibration complete");

    if (g_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        g_bNeedPose = TRUE;
        if (!g_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            printf("Pose required, but not supported\n");
//            return 1;
        }
        nRetVal = g_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(nRetVal, "Register to Pose Detected");
        g_UserGenerator.GetSkeletonCap().GetCalibrationPose(g_strPose);
    }

    XnUInt32    nNameLength=100;
    XnUInt32   	nPoses=nNameLength;
    XnChar *  	pstrPoses = new XnChar[nNameLength];
    memset(pstrPoses,0,nNameLength);

    printf("NumberOfPoses = %d\n",g_UserGenerator.GetPoseDetectionCap().GetNumberOfPoses());
    g_UserGenerator.GetPoseDetectionCap().GetAvailablePoses(&pstrPoses,nPoses);
    printf("POSES = ");
    for (int i=0; i<nNameLength; i++)
        printf("%c",pstrPoses[i]);
   printf("|\n");
       g_UserGenerator.GetPoseDetectionCap().GetAvailablePoses(&pstrPoses,nPoses);
    printf("POSES = ");
    for (int i=0; i<nNameLength; i++)
        printf("%c",pstrPoses[i]);
   printf("|\n");
    g_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

    nRetVal = g_Context.StartGeneratingAll();
    CHECK_RC(nRetVal, "StartGenerating");

    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation torsoJoint;

    printf("Starting to run\n");
    if(g_bNeedPose)
    {
        printf("Assume calibration pose\n");
    }
    XnUInt32 epochTime = 0;
    while (!xnOSWasKeyboardHit())
    {
        g_Context.WaitOneUpdateAll(g_UserGenerator);
        // print the torso information for the first user already tracking
        nUsers=MAX_NUM_USERS;
        g_UserGenerator.GetUsers(aUsers, nUsers);
        int numTracked=0;
        int userToPrint=-1;
        for(XnUInt16 i=0; i<nUsers; i++)
        {
            if(g_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==FALSE)
                continue;

            g_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_TORSO,torsoJoint);
//                printf("user %d: head at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
//                                                                torsoJoint.position.position.X,
//                                                                torsoJoint.position.position.Y,
//                                                                torsoJoint.position.position.Z);
        }
        
    }
    g_scriptNode.Release();
    g_UserGenerator.Release();
    g_Context.Release();
}


UKinectUser::UKinectUser(const std::string& name) : UKinectModule(name) {
    UBindFunction(UKinectUser, init);
}

UKinectUser::~UKinectUser() {
    userGenerator.Release();
}

void UKinectUser::init() {
    cerr << "UKinectUser::init()" << endl;
    
     mainThread = boost::thread(&UKinectUser::mainThreadFunction, this);
     return;
    
    // Urbi constructor
    mGetNewFrame = true;

    XnStatus nRetVal = XN_STATUS_OK;

    // initialize generator
    nRetVal = userGenerator.Create(OpenNIKinectSingelton::getInstance().getContext());
    if (nRetVal != XN_STATUS_OK)
        throw runtime_error("Failed to initialize camera");

    // Bind all variables
    UBindVar(UKinectUser, fps);
    UBindVar(UKinectUser, notify);
    UBindVar(UKinectUser, usersCount);

    // Bind all functions
    UBindThreadedFunction(UKinectUser, getData, LOCK_INSTANCE);

    // Notify if fps changed
    UNotifyChange(fps, &UKinectUser::fpsChanged);
    UNotifyChange(notify, &UKinectUser::changeNotifyImage);

    //UNotifyAccess(usersCount, &UKinectUser::getData);
    
    // Set update period 30
    fps = 30;
    OpenNIKinectSingelton::getInstance().getContext().StartGeneratingAll();

    nUsers = MAX_NUM_USERS;
}

/* 
 * mode = 0 - slave mode in multi generator work
 * mode = 1 - master mode in multi generator work
 * mode = 2 - single mode
 */
void UKinectUser::getData(int mode) {
    // Lock access to this method from urbi
    boost::lock_guard<boost::mutex> lock(getValMutex);
    XnStatus nRetVal = XN_STATUS_OK;
    nUsers = MAX_NUM_USERS;
    // If there is new frame
    if (mGetNewFrame) {
        mGetNewFrame = false;
        if (mode == 1) {
            nRetVal = OpenNIKinectSingelton::getInstance().getContext().WaitAndUpdateAll();
        }
        if (mode == 2) {
            nRetVal = OpenNIKinectSingelton::getInstance().getContext().WaitOneUpdateAll(userGenerator);
        }
        if (nRetVal != XN_STATUS_OK) {
            cerr << "UKinectUser::getData() - contex.WaitOneUpdateAll error" << endl;
            return;
        } else {
            usersSkeleton.clear();
            userGenerator.GetUsers(aUsers, nUsers);
            usersCount = nUsers;
        }
    }
}

std::vector<float> UKinectUser::setVectorPosition(unsigned int user, unsigned int jointNumber, XnSkeletonJoint eJoint) {
    std::vector<float> position;
    XnSkeletonJointPosition joint;
    userGenerator.GetSkeletonCap().GetSkeletonJointPosition(aUsers[user], XN_SKEL_HEAD, joint);
    position.push_back(joint.position.X);
    position.push_back(joint.position.Y);
    position.push_back(joint.position.Z);
    return position;
}

std::vector<float> UKinectUser::getJointPosition(unsigned int user, unsigned int jointNumber) {
    std::vector<float> position;
    if (user>=nUsers)
        return position;
    if (userGenerator.GetSkeletonCap().IsTracking(aUsers[user]) == FALSE)
        return position;
    if (jointNumber>24)
        return position;
    switch (jointNumber) {
        case 1:
            return setVectorPosition(user, jointNumber, XN_SKEL_HEAD);
        case 2:
            return setVectorPosition(user, jointNumber, XN_SKEL_NECK);
        case 3:
            return setVectorPosition(user, jointNumber, XN_SKEL_TORSO);
        case 4:
            return setVectorPosition(user, jointNumber, XN_SKEL_WAIST);
        case 5:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_COLLAR);         
        case 6:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_SHOULDER);            
        case 7:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_ELBOW);        
        case 8:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_WRIST);            
        case 9:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_HAND);           
        case 10:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_FINGERTIP);           
        case 11:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_COLLAR);          
        case 12:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_SHOULDER);         
        case 13:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_ELBOW);           
        case 14:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_WRIST);           
        case 15:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_HAND);          
        case 16:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_FINGERTIP);         
        case 17:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_HIP);          
        case 18:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_KNEE);            
        case 19:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_ANKLE);           
        case 20:
            return setVectorPosition(user, jointNumber, XN_SKEL_LEFT_FOOT);           
        case 21:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_HIP);     
        case 22:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_KNEE);           
        case 23:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_ANKLE);            
        case 24:
            return setVectorPosition(user, jointNumber, XN_SKEL_RIGHT_FOOT);                               
    }
    return position;
}

void UKinectUser::changeNotifyImage(UVar & var) {
    // Always unnotify
    usersCount.unnotify();
    if (var.as<bool>())
        UNotifyAccess(usersCount, &UKinectUser::getData);
}

int UKinectUser::update() {
    mGetNewFrame = true;
    return 0;
}

void UKinectUser::fpsChanged() {
    cerr << "UKinectUser::fpsChanged()" << endl
            << "\tCamera fps changed to " << fps.as<int>() << endl;
    USetUpdate(fps.as<int>() > 0 ? 1000.0 / fps.as<int>() : -1.0);
}

UStart(UKinectUser);