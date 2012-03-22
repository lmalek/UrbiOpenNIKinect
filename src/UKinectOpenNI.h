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
    
    urbi::UVar numUsers;

    urbi::UVar fps;
    urbi::UVar notify;

    void refreshData();

    void getImage();
    void getDepth();
    void getUsers();

    void changeNotifyImage(urbi::UVar & var);
    void changeNotifyDepth(urbi::UVar & var);
    void changeNotifyUsers(urbi::UVar & var);
    
    bool isUserTracked(unsigned int nr);
    std::vector<float> getJointPosition(unsigned int user, unsigned int jointNumber);

    unsigned int getDepthXY(unsigned int x, unsigned int y);

    unsigned int getDepthMedianFromArea(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

private:

    bool imageActive;
    bool depthActive;
    bool userActive;

    bool mGetNewData; //
    unsigned int mData; // ID of already grabed frame
    unsigned int mAccessData; // ID of already retrieved frame  

    // Storage for last captured image.
    urbi::UBinary mBinImage;
    urbi::UBinary mBinDepth;

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    void fpsChanged();

    xn::ImageGenerator imageGenerator;
    xn::ImageMetaData imageMD;

    xn::DepthGenerator depthGenerator;
    xn::DepthMetaData depthMD;

    xn::UserGenerator userGenerator;

    xn::Context context;
    
    XnUInt16 nUsers;
    XnUserID aUsers[MAX_NUM_USERS];
    std::vector<float> setVectorPosition(unsigned int user, XnSkeletonJoint eJoint);  
};

#endif	/* UKINECTOPENNI_H */

