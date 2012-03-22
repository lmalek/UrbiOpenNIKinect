/* 
 * File:   UKinectOpenNI.h
 * Author: lmalek
 *
 * Created on 21 marzec 2012, 09:55
 */

#ifndef UKINECTOPENNI_H
#define	UKINECTOPENNI_H

#include <urbi/uobject.hh>
#include <XnCppWrapper.h>
#include <string>

#include <cv.h>

#define MAX_NUM_USERS 10

class UKinectOpenNI : public urbi::UObject {
public:
    UKinectOpenNI(const std::string& name);
    virtual ~UKinectOpenNI();

    virtual int update();

    void init(bool image, bool depth, bool user);

    void activateImage();
    void deactivateImage();

    void activateDepth();
    void deactivateDepth();

    void activateUsers();
    void deactivateUsers();

    urbi::UVar image;
    urbi::UVar imageWidth;
    urbi::UVar imageHeight;

    urbi::UVar depth;
    urbi::UVar depthWidth;
    urbi::UVar depthHeight;

    urbi::UVar skeleton;
    urbi::UVar skeletonWidth;
    urbi::UVar skeletonHeight;
    urbi::UVar numUsers;

    urbi::UVar fps;
    urbi::UVar notify;

    void refreshData();

    // image component functions
    void getImage();
    void changeNotifyImage(urbi::UVar & var);

    // depth component functions
    void getDepth();
    void changeNotifyDepth(urbi::UVar & var);
    unsigned int getDepthXY(unsigned int x, unsigned int y);
    unsigned int getDepthMedianFromArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

    // users component functions
    void getUsers();
    void getSkeleton(urbi::UImage src);
    void changeNotifyUsers(urbi::UVar & var);
    bool isUserTracked(unsigned int nr);
    std::vector<float> getJointPosition(unsigned int user, unsigned int jointNumber); 
   
private:


    bool mGetNewData; //
    unsigned int mData; // ID of already grabed frame
    unsigned int mAccessData; // ID of already retrieved frame  

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    void fpsChanged();

    xn::Context context;

    // image component variables
    bool imageActive;
    xn::ImageGenerator imageGenerator;
    xn::ImageMetaData imageMD;
    urbi::UBinary mBinImage; // Storage for last captured image.

    // depth component variables
    bool depthActive;
    xn::DepthGenerator depthGenerator;
    xn::DepthMetaData depthMD;
    urbi::UBinary mBinDepth; // Storage for last captured image.

    // users component variables
    bool usersActive;
    xn::UserGenerator userGenerator;
    urbi::UBinary mBinSkeleton; // Storage for last captured image.
    XnUInt16 nUsers;
    XnUserID aUsers[MAX_NUM_USERS];
    cv::Mat skeletonImage;

    // user component functions
    std::vector<float> setVectorPosition(unsigned int user, XnSkeletonJoint eJoint);
};

#endif	/* UKINECTOPENNI_H */

