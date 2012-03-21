/* 
 * File:   UKinectUser.h
 * Author: lmalek
 *
 * Created on 19 marzec 2012, 11:13
 */

#ifndef UKINECTUSER_H
#define	UKINECTUSER_H

#include "UKinectModule.h"
#include <map>
#include <vector>

#define MAX_NUM_USERS 15

class UKinectUser : public UKinectModule{
    typedef std::map<XnSkeletonJoint,XnSkeletonJointTransformation> KinectSkeleton;
public:
    // Urbi constructor. Throw error in case of error.
    UKinectUser(const std::string& name);
    virtual ~UKinectUser();

    virtual int update();

    void init();
private:
    // Our image variable and dimensions
    urbi::UVar fps;
    urbi::UVar notify;
    urbi::UVar usersCount;

    bool mGetNewFrame; //
    unsigned int mFrame; // ID of already grabed frame
    unsigned int mAccessFrame; // ID of already retrieved frame

    // Called on access.
    /* 
     * mode = 0 - slave mode in multi generator work
     * mode = 1 - master mode in multi generator work
     * mode = 2 - single mode
     */
    void getData(int mode=0);
    
    std::vector<float> setVectorPosition(unsigned int user, unsigned int jointNumber, XnSkeletonJoint eJoint);
    
    std::vector<float> getJointPosition(unsigned int user, unsigned int jointNumber);
    
    //
    void changeNotifyImage(urbi::UVar&);

    // Mutex and conditional variable to synchronize threads
    boost::mutex getValMutex;

    // Storage for last captured image.
    urbi::UBinary mBinImage;

    void fpsChanged();

    xn::UserGenerator userGenerator;
    
    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    std::map<int, KinectSkeleton> usersSkeleton;
    
    void mainThreadFunction();
};

#endif	/* UKINECTUSER_H */

